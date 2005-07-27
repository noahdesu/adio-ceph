/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

/** @file **/

#include "orte_config.h"

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "include/constants.h"
#include "opal/event/event.h"
#include "opal/util/output.h"
#include "opal/threads/mutex.h"
#include "opal/runtime/opal.h"
#include "dps/dps.h"
#include "mca/mca.h"
#include "mca/base/base.h"
#include "mca/base/mca_base_param.h"
#include "mca/rml/base/base.h"
#include "mca/errmgr/base/base.h"
#include "mca/iof/base/base.h"
#include "mca/ns/base/base.h"
#include "mca/sds/base/base.h"
#include "mca/gpr/base/base.h"
#include "mca/rmgr/base/base.h"
#include "mca/rmaps/base/base.h"
#include "mca/schema/base/base.h"
#include "mca/soh/base/base.h"
#include "opal/util/malloc.h"
#include "util/univ_info.h"
#include "util/proc_info.h"
#include "util/session_dir.h"
#include "util/sys_info.h"
#include "opal/util/cmd_line.h"
#include "util/universe_setup_file_io.h"
#include "opal/util/os_path.h"

#include "runtime/runtime.h"
#include "runtime/runtime_internal.h"
#include "runtime/orte_wait.h"

int orte_init_stage1(void)
{
    int ret, rc;
    char *jobid_str = NULL;
    char *procid_str = NULL;
    char *contact_path = NULL;
    orte_jobid_t my_jobid;
    orte_cellid_t my_cellid;
    orte_gpr_value_t value, *values;

    /* Ensure the system_info structure is instantiated and initialized */
    if (ORTE_SUCCESS != (ret = orte_sys_info())) {
        return ret;
    }

    /* Ensure the process info structure is instantiated and initialized */
    if (ORTE_SUCCESS != (ret = orte_proc_info())) {
        return ret;
    }
    
    /* Ensure the universe_info structure is instantiated and initialized */
    if (ORTE_SUCCESS != (ret = orte_univ_info())) {
        return ret;
    }

    /*
     * Initialize the data packing service.
     */
    if (ORTE_SUCCESS != (ret = orte_dps_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
    
    /*
     * Open the name services to ensure access to local functions 
     */
    if (OMPI_SUCCESS != (ret = orte_ns_base_open())) {
        return ret;
    }

    /* Open the error manager to activate error logging - needs local name services */
    if (ORTE_SUCCESS != (ret = orte_errmgr_base_open())) {
        return ret;
    }
    
    /*****   ERROR LOGGING NOW AVAILABLE *****/
    
    /* check for debug flag */
    if (0 > (ret =  mca_base_param_register_int("orte", "debug", NULL, NULL, 0))) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
    if (ORTE_SUCCESS != (ret = mca_base_param_lookup_int(ret, &orte_debug_flag))) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Initialize the event library 
    */
    if (OMPI_SUCCESS != (ret = opal_event_init())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Intialize the general progress engine
     */
    if (OMPI_SUCCESS != (ret = opal_progress_init())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Internal startup
     */
    if (OMPI_SUCCESS != (ret = orte_wait_init())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Runtime Messaging Layer
     */
    if (OMPI_SUCCESS != (ret = orte_rml_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Runtime Messaging Layer
     */
    if (OMPI_SUCCESS != (ret = orte_rml_base_select())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Registry
     */
    if (ORTE_SUCCESS != (ret = orte_gpr_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Initialize schema utilities
     */
    if (ORTE_SUCCESS != (ret = orte_schema_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Initialize and select sds
     */
    if (ORTE_SUCCESS != (ret = orte_sds_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
    if (ORTE_SUCCESS != (ret = orte_sds_base_select())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /* Try to connect to the universe */
    if (ORTE_SUCCESS != (ret = orte_sds_base_contact_universe())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Name Server 
     */
    if (OMPI_SUCCESS != (ret = orte_ns_base_select())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /*
     * Registry 
     */
    if (ORTE_SUCCESS != (ret = orte_gpr_base_select())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
 
    /* set contact info for ns/gpr */
    if(NULL != orte_process_info.ns_replica_uri) {
        orte_rml.set_uri(orte_process_info.ns_replica_uri);
    }
    if(NULL != orte_process_info.gpr_replica_uri) {
        orte_rml.set_uri(orte_process_info.gpr_replica_uri);
    }

    /* set my name and the names of the procs I was started with */
    if (ORTE_SUCCESS != (ret = orte_sds_base_set_name())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    /* all done with sds - clean up and call it a day */
    orte_sds_base_close();

    /* if I'm the seed, set the seed uri to be me! */
    if (orte_process_info.seed) {
        orte_universe_info.seed_uri = orte_rml.get_uri();
    }
    
    /* setup my session directory */
    if (ORTE_SUCCESS != (ret = orte_ns.get_jobid_string(&jobid_str, orte_process_info.my_name))) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
    if (ORTE_SUCCESS != (ret = orte_ns.get_vpid_string(&procid_str, orte_process_info.my_name))) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }
 
    if (orte_debug_flag) {
        opal_output(0, "[%lu,%lu,%lu] setting up session dir with",
                    ORTE_NAME_ARGS(orte_process_info.my_name));
        if (NULL != orte_process_info.tmpdir_base) {
            opal_output(0, "\ttmpdir %s", orte_process_info.tmpdir_base);
        }
        opal_output(0, "\tuniverse %s", orte_universe_info.name);
        opal_output(0, "\tuser %s", orte_system_info.user);
        opal_output(0, "\thost %s", orte_system_info.nodename);
        opal_output(0, "\tjobid %s", jobid_str);
        opal_output(0, "\tprocid %s", procid_str);
    }
    if (ORTE_SUCCESS != (ret = orte_session_dir(true,
                                orte_process_info.tmpdir_base,
                                orte_system_info.user,
                                orte_system_info.nodename, NULL,
                                orte_universe_info.name,
                                jobid_str, procid_str))) {
        if (jobid_str != NULL) free(jobid_str);
        if (procid_str != NULL) free(procid_str);
        ORTE_ERROR_LOG(ret);
        return ret;
    }
    if (NULL != jobid_str) {
        free(jobid_str);
    }
    if (NULL != procid_str) {
        free(procid_str);
    }

    /* if i'm the seed, get my contact info and write my setup file for others to find */
    if (orte_process_info.seed) {
        if (NULL != orte_universe_info.seed_uri) {
            free(orte_universe_info.seed_uri);
            orte_universe_info.seed_uri = NULL;
        }
        if (NULL == (orte_universe_info.seed_uri = orte_rml.get_uri())) {
            ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
            return ORTE_ERR_NOT_FOUND;
        }
        contact_path = opal_os_path(false, orte_process_info.universe_session_dir,
                    "universe-setup.txt", NULL);
        if (orte_debug_flag) {
            opal_output(0, "[%lu,%lu,%lu] contact_file %s",
                        ORTE_NAME_ARGS(orte_process_info.my_name), contact_path);
        }

        if (OMPI_SUCCESS != (ret = orte_write_universe_setup_file(contact_path, &orte_universe_info))) {
            if (orte_debug_flag) {
                opal_output(0, "[%lu,%lu,%lu] couldn't write setup file", ORTE_NAME_ARGS(orte_process_info.my_name));
            }
        } else if (orte_debug_flag) {
            opal_output(0, "[%lu,%lu,%lu] wrote setup file", ORTE_NAME_ARGS(orte_process_info.my_name));
        }
        free(contact_path);
    }

    /* open/load rmgr/soh */

    if (ORTE_SUCCESS != (ret = orte_rmgr_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

    if (ORTE_SUCCESS != (ret = orte_soh_base_open())) {
        ORTE_ERROR_LOG(ret);
        return ret;
    }

     /* if we are a singleton or the seed, setup the infrastructure for our job */
 
    if(orte_process_info.singleton || orte_process_info.seed) {
        if (ORTE_SUCCESS != (ret = orte_ns.get_jobid(&my_jobid, orte_process_info.my_name))) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
        if (ORTE_SUCCESS != (ret = orte_ns.get_cellid(&my_cellid, orte_process_info.my_name))) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
        if (orte_process_info.singleton) {
            /* setup a fake node structure - this is required to support
             * the MPI attributes function that is sitting on a trigger
             * waiting for info on available node slots. since we can't
             * really know that info for a singleton, we make the assumption
             * that the allocation is unity and place a structure on the
             * registry for it
             * 
             * THIS ONLY SHOULD BE DONE FOR SINGLETONS - DO NOT DO IT
             * FOR ANY OTHER CASE
             */
            OBJ_CONSTRUCT(&value, orte_gpr_value_t);
            values = &value;
            /* define the addressing mode and segment */
            value.addr_mode = ORTE_GPR_TOKENS_OR | ORTE_GPR_KEYS_OR;
            value.segment = strdup(ORTE_NODE_SEGMENT);
            if (NULL == value.segment) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            if (ORTE_SUCCESS != (rc = orte_schema.get_node_tokens(&(value.tokens),
                    &(value.num_tokens), my_cellid, orte_system_info.nodename))) {
                ORTE_ERROR_LOG(rc);
                return rc;
            }
            value.cnt = 2;

            value.keyvals = (orte_gpr_keyval_t**)malloc(2*sizeof(orte_gpr_keyval_t*));
            if (NULL == value.keyvals) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            value.keyvals[0] = OBJ_NEW(orte_gpr_keyval_t);
            if (NULL == value.keyvals[0]) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            value.keyvals[0]->key = strdup(ORTE_NODE_SLOTS_KEY);
            if (NULL == value.keyvals[0]->key) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            value.keyvals[0]->type = ORTE_SIZE;
            value.keyvals[0]->value.size = 1;

            value.keyvals[1] = OBJ_NEW(orte_gpr_keyval_t);
            if (NULL == value.keyvals[1]) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            value.keyvals[1]->key = strdup(ORTE_NODE_NAME_KEY);
            if (NULL == value.keyvals[1]->key) {
                ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
                OBJ_DESTRUCT(&value);
                return ORTE_ERR_OUT_OF_RESOURCE;
            }
            value.keyvals[1]->type = ORTE_STRING;
            value.keyvals[1]->value.strptr = strdup(orte_system_info.nodename);
            /* put the value on the registry */
            if (ORTE_SUCCESS != (ret = orte_gpr.put(1, &values))) {
                ORTE_ERROR_LOG(ret);
                OBJ_DESTRUCT(&value);
                return ret;
            }
            /* cleanup the mess */
            OBJ_DESTRUCT(&value);
        }
        
        /* set the rest of the infrastructure */
        if (ORTE_SUCCESS != (ret = orte_rmgr_base_set_job_slots(my_jobid,1))) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
        if (ORTE_SUCCESS != (ret = orte_rmaps_base_set_vpid_range(my_jobid,0,1))) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
        if (ORTE_SUCCESS != (ret = orte_rmgr_base_proc_stage_gate_init(my_jobid))) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
    }

    return ORTE_SUCCESS;
}
