/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 *
 */

#include "orte_config.h"
#include "orte/constants.h"

#include <catamount/cnos_mpi_os.h>

#include "orte/util/show_help.h"
#include "opal/util/argv.h"
#include "opal/class/opal_pointer_array.h"
#include "opal/mca/paffinity/paffinity.h"

#include "orte/util/proc_info.h"
#include "orte/mca/errmgr/base/base.h"
#include "orte/util/name_fns.h"
#include "orte/util/nidmap.h"
#include "orte/runtime/orte_globals.h"

#include "orte/mca/ess/ess.h"
#include "orte/mca/ess/base/base.h"
#include "orte/mca/ess/alps/ess_alps.h"

static int alps_set_name(void);

static int rte_init(char flags);
static int rte_finalize(void);
static uint8_t proc_get_locality(orte_process_name_t *proc);
static orte_vpid_t proc_get_daemon(orte_process_name_t *proc);
static char* proc_get_hostname(orte_process_name_t *proc);
static uint32_t proc_get_arch(orte_process_name_t *proc);
static orte_local_rank_t proc_get_local_rank(orte_process_name_t *proc);
static orte_node_rank_t proc_get_node_rank(orte_process_name_t *proc);
static int update_arch(orte_process_name_t *proc, uint32_t arch);
static int update_pidmap(opal_byte_object_t *bo);
static int update_nidmap(opal_byte_object_t *bo);

orte_ess_base_module_t orte_ess_alps_module = {
    rte_init,
    rte_finalize,
    orte_ess_base_app_abort,
    proc_get_locality,
    proc_get_daemon,
    proc_get_hostname,
    proc_get_arch,
    proc_get_local_rank,
    proc_get_node_rank,
    update_arch,
    update_pidmap,
    update_nidmap,
    NULL /* ft_event */
};


static int rte_init(char flags)
{
    int ret;
    char *error = NULL;

    /* run the prolog */
    if (ORTE_SUCCESS != (ret = orte_ess_base_std_prolog())) {
        error = "orte_ess_base_std_prolog";
        goto error;
    }
    
    /* Start by getting a unique name */
    alps_set_name();
    
    /* if I am a daemon, complete my setup using the
     * default procedure
     */
    if (orte_proc_info.daemon) {
        if (ORTE_SUCCESS != (ret = orte_ess_base_orted_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_orted_setup";
            goto error;
        }
    } else if (orte_proc_info.tool) {
        /* otherwise, if I am a tool proc, use that procedure */
        if (ORTE_SUCCESS != (ret = orte_ess_base_tool_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_tool_setup";
            goto error;
        }
        /* as a tool, I don't need a nidmap - so just return now */
        return ORTE_SUCCESS;
    } else {
        /* otherwise, I must be an application process - use
        * the default procedure to finish my setup
        */
        if (ORTE_SUCCESS != (ret = orte_ess_base_app_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_app_setup";
            goto error;
        }
    }
    
    /* setup the nidmap arrays */
    if (ORTE_SUCCESS != (ret = orte_util_nidmap_init(orte_proc_info.sync_buf))) {
        ORTE_ERROR_LOG(ret);
        error = "orte_util_nidmap_init";
        goto error;
    }
    
    return ORTE_SUCCESS;
    
error:
        orte_show_help("help-orte-runtime.txt",
                       "orte_init:startup:internal-failure",
                       true, error, ORTE_ERROR_NAME(ret), ret);
    
    return ret;
}

static int rte_finalize(void)
{
    int ret;
    
    /* if I am a daemon, finalize using the default procedure */
    if (orte_proc_info.daemon) {
        if (ORTE_SUCCESS != (ret = orte_ess_base_orted_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
    } else if (orte_proc_info.tool) {
        /* otherwise, if I am a tool proc, use that procedure */
        if (ORTE_SUCCESS != (ret = orte_ess_base_tool_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
        /* as a tool, I didn't create a nidmap - so just return now */
        return ret;
    } else {
        /* otherwise, I must be an application process
         * use the default procedure to finish
         */
        if (ORTE_SUCCESS != (ret = orte_ess_base_app_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
    }
    
    
    /* deconstruct my nidmap and jobmap arrays */
    orte_util_nidmap_finalize();
    
    return ret;    
}

static uint8_t proc_get_locality(orte_process_name_t *proc)
{
    orte_nid_t *nid;
    
    if (NULL == (nid = orte_util_lookup_nid(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return OPAL_PROC_NON_LOCAL;
    }
    
    if (nid->daemon == ORTE_PROC_MY_DAEMON->vpid) {
        OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                             "%s ess:alps: proc %s on LOCAL NODE",
                             orte_util_print_name_args(ORTE_PROC_MY_NAME),
                             orte_util_print_name_args(proc)));
        return (OPAL_PROC_ON_NODE | OPAL_PROC_ON_CU | OPAL_PROC_ON_CLUSTER);
    }
    
    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s is REMOTE",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc)));
    
    return OPAL_PROC_NON_LOCAL;
    
}

static orte_vpid_t proc_get_daemon(orte_process_name_t *proc)
{
    orte_nid_t *nid;
    
    if (NULL == (nid = orte_util_lookup_nid(proc))) {
        return ORTE_VPID_INVALID;
    }
    
    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s is hosted by daemon %s",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         orte_util_print_vpids(nid->daemon)));
    
    return nid->daemon;
}

static char* proc_get_hostname(orte_process_name_t *proc)
{
    orte_nid_t *nid;
    
    if (NULL == (nid = orte_util_lookup_nid(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return NULL;
    }

    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s is on host %s",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         nid->name));
    
    return nid->name;
}

static uint32_t proc_get_arch(orte_process_name_t *proc)
{
    orte_nid_t *nid;
    
    if (NULL == (nid = orte_util_lookup_nid(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return 0;
    }

    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s has arch %0x",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         nid->arch));
    
    return nid->arch;
}

static int update_arch(orte_process_name_t *proc, uint32_t arch)
{
    orte_nid_t *nid;
    
    if (NULL == (nid = orte_util_lookup_nid(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    
    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: updating proc %s to arch %0x",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         arch));
    
    nid->arch = arch;
    
    return ORTE_SUCCESS;
}

static orte_local_rank_t proc_get_local_rank(orte_process_name_t *proc)
{
    orte_pmap_t *pmap;
    
    if (NULL == (pmap = orte_util_lookup_pmap(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_LOCAL_RANK_INVALID;
    }    

    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s has local rank %d",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         (int)pmap->local_rank));
    
    return pmap->local_rank;
}

static orte_node_rank_t proc_get_node_rank(orte_process_name_t *proc)
{
    orte_pmap_t *pmap;
    
    if (NULL == (pmap = orte_util_lookup_pmap(proc))) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_NODE_RANK_INVALID;
    }    
    
    OPAL_OUTPUT_VERBOSE((2, orte_ess_base_output,
                         "%s ess:alps: proc %s has node rank %d",
                         orte_util_print_name_args(ORTE_PROC_MY_NAME),
                         orte_util_print_name_args(proc),
                         (int)pmap->node_rank));
    
    return pmap->node_rank;
}

static int update_pidmap(opal_byte_object_t *bo)
{
    int ret;
    
    /* build the pmap */
    if (ORTE_SUCCESS != (ret = orte_util_decode_pidmap(bo))) {
        ORTE_ERROR_LOG(ret);
    }
    
    return ret;
}

static int update_nidmap(opal_byte_object_t *bo)
{
    int rc;
    /* decode the nidmap - the util will know what to do */
    if (ORTE_SUCCESS != (rc = orte_util_decode_nodemap(bo))) {
        ORTE_ERROR_LOG(rc);
    }    
    return rc;
}

static int alps_set_name(void)
{
    int rc;
    int id;
    orte_jobid_t jobid;
    orte_vpid_t starting_vpid;
    char* jobid_string;
    char* vpid_string;
    
    OPAL_OUTPUT_VERBOSE((1, orte_ess_base_output,
                         "ess:alps setting name"));
    
    id = mca_base_param_register_string("orte", "ess", "jobid", NULL, NULL);
    mca_base_param_lookup_string(id, &jobid_string);
    if (NULL == jobid_string) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    if (ORTE_SUCCESS != (rc = orte_util_convert_string_to_jobid(&jobid, jobid_string))) {
        ORTE_ERROR_LOG(rc);
        return(rc);
    }
    
    id = mca_base_param_register_string("orte", "ess", "vpid", NULL, NULL);
    mca_base_param_lookup_string(id, &vpid_string);
    if (NULL == vpid_string) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    if (ORTE_SUCCESS != (rc = orte_util_convert_string_to_vpid(&starting_vpid, vpid_string))) {
        ORTE_ERROR_LOG(rc);
        return(rc);
    }
    
    ORTE_PROC_MY_NAME->jobid = jobid;
    ORTE_PROC_MY_NAME->vpid = (orte_vpid_t) cnos_get_rank() + starting_vpid;
    
    OPAL_OUTPUT_VERBOSE((1, orte_ess_base_output,
                         "ess:alps set name to %s", orte_util_print_name_args(ORTE_PROC_MY_NAME)));
    
    orte_proc_info.num_procs = (orte_std_cntr_t) cnos_get_size();

    return ORTE_SUCCESS;
}
