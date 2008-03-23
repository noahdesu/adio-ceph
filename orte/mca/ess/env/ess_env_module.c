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

#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "opal/event/event.h"
#include "opal/util/output.h"
#include "opal/util/show_help.h"
#include "opal/threads/mutex.h"
#include "opal/runtime/opal.h"
#include "opal/runtime/opal_cr.h"

#include "opal/mca/mca.h"
#include "opal/mca/base/base.h"
#include "opal/mca/base/mca_base_param.h"
#include "opal/util/os_path.h"
#include "opal/util/cmd_line.h"
#include "opal/util/malloc.h"

#include "orte/mca/rml/base/base.h"
#include "orte/mca/rml/base/rml_contact.h"
#include "orte/mca/routed/base/base.h"
#include "orte/mca/routed/routed.h"
#include "orte/mca/errmgr/base/base.h"
#include "orte/mca/grpcomm/base/base.h"
#include "orte/mca/iof/base/base.h"
#include "orte/mca/ess/base/base.h"
#include "orte/mca/ess/ess.h"
#include "orte/mca/ras/base/base.h"
#include "orte/mca/plm/base/base.h"
#include "orte/mca/odls/base/base.h"

#include "orte/mca/rmaps/base/base.h"
#if OPAL_ENABLE_FT == 1
#include "orte/mca/snapc/base/base.h"
#endif
#include "orte/mca/filem/base/base.h"
#include "orte/util/proc_info.h"
#include "orte/util/session_dir.h"
#include "orte/util/hnp_contact.h"
#include "orte/util/name_fns.h"

#include "orte/runtime/runtime.h"
#include "orte/runtime/orte_wait.h"
#include "orte/runtime/orte_globals.h"

#include "orte/runtime/orte_cr.h"
#include "orte/mca/ess/ess.h"
#include "orte/mca/ess/base/base.h"
#include "orte/mca/ess/env/ess_env.h"

static int env_set_name(void);

static int rte_init(char flags);
static int rte_finalize(void);
#if OPAL_ENABLE_FT == 1
static int rte_ft_event(int state);
static int ess_env_ft_event_update_process_info(orte_process_name_t proc, pid_t pid);
#endif

orte_ess_base_module_t orte_ess_env_module = {
    rte_init,
    rte_finalize,
    orte_ess_base_app_abort,
#if OPAL_ENABLE_FT == 1
    rte_ft_event
#else
    NULL
#endif
};


static int rte_init(char flags)
{
    int ret;
    char *error = NULL;
    
    /* Start by getting a unique name from the enviro */
    env_set_name();

    /* if I am a daemon, complete my setup using the
     * default procedure
     */
    if (orte_process_info.daemon) {
        if (ORTE_SUCCESS != (ret = orte_ess_base_orted_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_orted_setup";
            goto error;
        }
        
    } else if (orte_process_info.tool) {
        /* otherwise, if I am a tool proc, use that procedure */
        if (ORTE_SUCCESS != (ret = orte_ess_base_tool_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_tool_setup";
            goto error;
        }
        
    } else {
        /* otherwise, I must be an application process, so
         * use that default procedure
         */
        if (ORTE_SUCCESS != (ret = orte_ess_base_app_setup())) {
            ORTE_ERROR_LOG(ret);
            error = "orte_ess_base_app_setup";
            goto error;
        }
        
    }

    return ORTE_SUCCESS;

error:
    opal_show_help("help-orte-runtime.txt",
                   "orte_init:startup:internal-failure",
                   true, error, ORTE_ERROR_NAME(ret), ret);
    
    return ret;
}

static int rte_finalize(void)
{
    int ret;
    
    /* if I am a daemon, finalize using the default procedure */
    if (orte_process_info.daemon) {
        if (ORTE_SUCCESS != (ret = orte_ess_base_orted_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
    } else if (orte_process_info.tool) {
        /* otherwise, if I am a tool proc, use that procedure */
        if (ORTE_SUCCESS != (ret = orte_ess_base_tool_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
    } else {
        /* otherwise, I must be an application process, so
         * use that default procedure
         */
        if (ORTE_SUCCESS != (ret = orte_ess_base_app_finalize())) {
            ORTE_ERROR_LOG(ret);
        }
    }
    
    return ret;    
}

static int env_set_name(void)
{
    char *jobid_str, *procid_str;
    int id, rc;
    orte_jobid_t jobid;
    orte_vpid_t vpid;
    
    id = mca_base_param_register_string("orte", "ess", "jobid", NULL, NULL);
    mca_base_param_lookup_string(id, &jobid_str);
    if (NULL == jobid_str) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    if (ORTE_SUCCESS != (rc = orte_util_convert_string_to_jobid(&jobid, jobid_str))) {
        ORTE_ERROR_LOG(rc);
        return(rc);
    }
    free(jobid_str);
    
    id = mca_base_param_register_string("orte", "ess", "vpid", NULL, NULL);
    mca_base_param_lookup_string(id, &procid_str);
    if (NULL == procid_str) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    if (ORTE_SUCCESS != (rc = orte_util_convert_string_to_vpid(&vpid, procid_str))) {
        ORTE_ERROR_LOG(rc);
        return(rc);
    }
    free(procid_str);
    
    ORTE_PROC_MY_NAME->jobid = jobid;
    ORTE_PROC_MY_NAME->vpid = vpid;
    
    OPAL_OUTPUT_VERBOSE((1, orte_ess_base_output,
                         "ess:env set name to %s", ORTE_NAME_PRINT(ORTE_PROC_MY_NAME)));
    
    /* get the non-name common environmental variables */
    if (ORTE_SUCCESS != (rc = orte_ess_env_get())) {
        ORTE_ERROR_LOG(rc);
        return rc;
    }

    return ORTE_SUCCESS;
}

#if OPAL_ENABLE_FT == 1
static int rte_ft_event(int state)
{
    int ret, exit_status = ORTE_SUCCESS;
    char * procid_str = NULL;
    char * jobid_str  = NULL;

    /******** Checkpoint Prep ********/
    if(OPAL_CRS_CHECKPOINT == state) {
        /*
         * Notify IOF
         */
        if( ORTE_SUCCESS != (ret = orte_iof.ft_event(OPAL_CRS_CHECKPOINT))) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * Notify RML & OOB
         */
        if( ORTE_SUCCESS != (ret = orte_rml.ft_event(OPAL_CRS_CHECKPOINT))) {
            exit_status = ret;
            goto cleanup;
        }
    }
    /******** Continue Recovery ********/
    else if (OPAL_CRS_CONTINUE == state ) {
        /*
         * Notify RML & OOB
         */
        if( ORTE_SUCCESS != (ret = orte_rml.ft_event(OPAL_CRS_CONTINUE))) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * Notify IOF
         */
        if( ORTE_SUCCESS != (ret = orte_iof.ft_event(OPAL_CRS_CONTINUE))) {
            exit_status = ret;
            goto cleanup;
        }
    }
    /******** Restart Recovery ********/
    else if (OPAL_CRS_RESTART == state ) {
        /*
         * Notify RML & OOB
         */
        if( ORTE_SUCCESS != (ret = orte_rml.ft_event(OPAL_CRS_RESTART))) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * - Reset Contact information
         */
        if( ORTE_SUCCESS != (ret = env_set_name() ) ) {
            exit_status = ret;
        }

        /* Session directory stuff:
         *   orte_process_info.top_session_dir
         *   orte_process_info.universe_session_dir
         *   orte_process_info.job_session_dir
         *   orte_process_info.proc_session_dir
         */
        if (ORTE_SUCCESS != (ret = orte_util_convert_jobid_to_string(&jobid_str, ORTE_PROC_MY_NAME->jobid))) {
            exit_status = ret;
        }

        if (ORTE_SUCCESS != (ret = orte_util_convert_vpid_to_string(&procid_str, ORTE_PROC_MY_NAME->vpid))) {
            exit_status = ret;
        }

        if (ORTE_SUCCESS != (ret = orte_session_dir(true,
                                                    orte_process_info.tmpdir_base,
                                                    orte_process_info.nodename,
                                                    NULL, /* Batch ID -- Not used */
                                                    jobid_str,
                                                    procid_str))) {
            exit_status = ret;
        }

        /*
         * Re-enable communication through the RML
         */
        if (ORTE_SUCCESS != (ret = orte_rml.enable_comm())) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * Notify IOF
         */
        if( ORTE_SUCCESS != (ret = orte_iof.ft_event(OPAL_CRS_RESTART))) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * Re-exchange the routes
         */
        if (ORTE_SUCCESS != (ret = orte_routed.initialize()) ) {
            exit_status = ret;
            goto cleanup;
        }
        if (ORTE_SUCCESS != (ret = orte_routed.init_routes(ORTE_PROC_MY_NAME->jobid, NULL))) {
            exit_status = ret;
            goto cleanup;
        }

        /*
         * Send new PID to HNP/daemon
         * The checkpointer could have used a proxy program to boot us
         * so the pid that the orted got from fork() may not be the
         * PID of this application.
         * - Note: BLCR does this because it tries to preseve the PID
         *         of the program across checkpointes
         */
        if( ORTE_SUCCESS != (ret = ess_env_ft_event_update_process_info(orte_process_info.my_name, getpid())) ) {
            exit_status = ret;
            goto cleanup;
        }
    }
    else if (OPAL_CRS_TERM == state ) {
        /* Nothing */
    }
    else {
        /* Error state = Nothing */
    }

 cleanup:
    if (NULL != jobid_str) {
        free(jobid_str);
        jobid_str = NULL;
    }

    return exit_status;
}

static int ess_env_ft_event_update_process_info(orte_process_name_t proc, pid_t proc_pid)
{
    int ret, exit_status = ORTE_SUCCESS;
    opal_buffer_t buffer;
    orte_snapc_cmd_flag_t command = ORTE_SNAPC_LOCAL_UPDATE_CMD;

    OBJ_CONSTRUCT(&buffer, opal_buffer_t);

    if (ORTE_SUCCESS != (ret = opal_dss.pack(&buffer, &command, 1, ORTE_SNAPC_CMD )) ) {
        ORTE_ERROR_LOG(ret);
        exit_status = ret;
        goto cleanup;
    }

    if (ORTE_SUCCESS != (ret = opal_dss.pack(&buffer, &proc, 1, ORTE_NAME))) {
        ORTE_ERROR_LOG(ret);
        exit_status = ret;
        goto cleanup;
    }

    if (ORTE_SUCCESS != (ret = opal_dss.pack(&buffer, &proc_pid, 1, OPAL_PID))) {
        ORTE_ERROR_LOG(ret);
        exit_status = ret;
        goto cleanup;
    }

    if (0 > (ret = orte_rml.send_buffer(ORTE_PROC_MY_DAEMON, &buffer, ORTE_RML_TAG_SNAPC, 0))) {
        ORTE_ERROR_LOG(ret);
        exit_status = ret;
        goto cleanup;
    }

 cleanup:
    OBJ_DESTRUCT(&buffer);

    return exit_status;
}
#endif

