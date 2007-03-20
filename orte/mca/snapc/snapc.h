/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
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
 */
/**
 * @file
 *
 * Snapshot Coordination (SNAPC) Interface
 *
 * Terminology:
 * ------------
 *  Global Snapshot Coordinator:
 *     - HNP(s) coordination function.
 *  Local Snapshot Coordinator
 *     - VHNP(s) [e.g., orted] coordination function
 *  Application Snapshot Coordinator
 *     - Application level coordinaton function
 *  Local Snapshot
 *     - Snapshot generated by a single process in the parallel job
 *  Local Snapshot Reference
 *     - A generic reference to the physical Local Snapshot 
 *  Global Snapshot
 *     - Snapshot generated for the entire parallel job
 *  Global Snapshot Reference
 *     - A generic reference to the physical Global Snapshot 
 *
 * General Description:
 * ---------------------
 * This framework is tasked with:
 * - Initiating the checkpoint in the system
 * - Physically moving the local snapshot files to a location
 *   Initially this location, is the node on which the Head Node Process (HNP)
 *   is running, but later this will be a replicated checkpoint server or
 *   the like.
 * - Generating a 'global snapshot handle' that the user can use to restart
 *   the parallel job.
 *
 * Each component will have 3 teirs of behavior that must behave in concert:
 *  - Global Snapshot Coordinator
 *    This is the HNPs tasks. Mostly distributing the notification of the
 *    checkpoint, and then compiling the physical and virtual nature of the
 *    global snapshot handle.
 *  - Local Snapshot Coordinator
 *    This is the VHNPs (or orted, if available) tasks. This will involve
 *    working with the Global Snapshot Coordinator to route the physical
 *    and virtual 'local snapshot's from the application to the desired
 *    location. This process must also notify the Global Snapshot Coordinator
 *    when it's set of processes have completed the checkpoint.
 *  - Application Snapshot Coordinator
 *    This is the application level coordinator. This is very light, just
 *    a subscription to be triggered when it needs to checkpoint, and then,
 *    once finished with the checkpoint, notify the Local Snapshot Coordinator
 *    that it is complete.
 *    If there is no orted (so no bootproxy), then the application assumes the
 *    responsibility of the Local Snapshot Coordinator as well.
 *
 */

#ifndef MCA_SNAPC_H
#define MCA_SNAPC_H

#include "orte_config.h"
#include "orte/orte_constants.h"
#include "orte/orte_types.h"

#include "opal/util/output.h"
#include "opal/mca/mca.h"
#include "opal/mca/base/base.h"
#include "opal/mca/crs/crs.h"
#include "opal/mca/crs/base/base.h"
#include "orte/mca/ns/ns.h"

#include "opal/class/opal_object.h"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

/**
 * Three types of Coordinator types, plus
 *  the case when it hasn't been defined.
 * These need to be able to be bit masked, so
 *  a process can be both Local & Application Coordinator 
 *  at the same time if needed.
 */
#define ORTE_SNAPC_UNASSIGN_TYPE     0
#define ORTE_SNAPC_GLOBAL_COORD_TYPE 1
#define ORTE_SNAPC_LOCAL_COORD_TYPE  2
#define ORTE_SNAPC_APP_COORD_TYPE    4

/**
 * States that a process can be in while checkpointing
 */
/* Doing no checkpoint -- Quiet state */
#define ORTE_SNAPC_CKPT_STATE_NONE          0
/* There has been a request for a checkpoint from one of the applications */
#define ORTE_SNAPC_CKPT_STATE_REQUEST       1
/* There is a Pending checkpoint for this process */
#define ORTE_SNAPC_CKPT_STATE_PENDING       2
/* There is a Pending checkpoint for this process, terminate the process after checkpoint */
#define ORTE_SNAPC_CKPT_STATE_PENDING_TERM  3
/* Running the checkpoint */
#define ORTE_SNAPC_CKPT_STATE_RUNNING       4
/* Finished the checkpoint */
#define ORTE_SNAPC_CKPT_STATE_FINISHED      5
/* Unable to checkpoint this job */
#define ORTE_SNAPC_CKPT_STATE_NO_CKPT       6
/* Reached an error */
#define ORTE_SNAPC_CKPT_STATE_ERROR         7

/**
 * Definition of a orte local snapshot.
 * Similar to the opal_crs_base_snapshot_t except that it
 * contains process contact information.
 */
struct orte_snapc_base_snapshot_1_0_0_t {
    opal_crs_base_snapshot_t crs_snapshot_super;
    /** ORTE Process name */
    orte_process_name_t process_name;
    /** PID of the application process that generated this snapshot */
    pid_t process_pid;
    /** State of the checkpoint */
    size_t state;
    /** Terminate this process after a checkpoint */
    bool term;
};
typedef struct orte_snapc_base_snapshot_1_0_0_t orte_snapc_base_snapshot_1_0_0_t;
typedef struct orte_snapc_base_snapshot_1_0_0_t orte_snapc_base_snapshot_t;

ORTE_DECLSPEC OBJ_CLASS_DECLARATION(orte_snapc_base_snapshot_t);

/**
 * Definition of the global snapshot.
 * Each component is assumed to have extened this definition
 * in the same way they extern the orte_snapc_base_compoinent_t below.
 */
struct orte_snapc_base_global_snapshot_1_0_0_t {
    /** This is an object, so must have super */
    opal_list_item_t super;

    /** A list of orte_snapc_base_snapshot_ts */
    opal_list_t snapshots;
    
    /* ORTE SnapC Component used to generate the global snapshot */
    char * component_name;

    /** Unique name of the global snapshot */
    char * reference_name;
    
    /** Location of the global snapshot Absolute path */
    char * local_location;
    
    /** Sequence Number */
    int seq_num;

    /** Beginning timestamp */
    char * start_time;

    /** Ending timestamp */
    char * end_time;
};
typedef struct orte_snapc_base_global_snapshot_1_0_0_t orte_snapc_base_global_snapshot_1_0_0_t;
typedef struct orte_snapc_base_global_snapshot_1_0_0_t orte_snapc_base_global_snapshot_t;

ORTE_DECLSPEC OBJ_CLASS_DECLARATION(orte_snapc_base_global_snapshot_t);

/**
 * Query function for SNAPC components.
 * Returns a priority to rank it agaianst other available SNAPC components.
 */
typedef struct orte_snapc_base_module_1_0_0_t *
        (*orte_snapc_base_component_query_1_0_0_fn_t)
        (int *priority);

/**
 * Module initialization function.
 * Returns ORTE_SUCCESS
 */
typedef int (*orte_snapc_base_module_init_fn_t)
     (bool seed, bool app);

/**
 * Module finalization function.
 * Returns ORTE_SUCCESS
 */
typedef int (*orte_snapc_base_module_finalize_fn_t)
     (void);

/**
 * Setup the necessary structures for this job
 * Returns ORTE_SUCCESS
 */
typedef int (*orte_snapc_base_setup_job_fn_t)
     (orte_jobid_t jobid);

/**
 * Setup the necessary structures for this job
 * Returns ORTE_SUCCESS
 */
typedef int (*orte_snapc_base_release_job_fn_t)
     (orte_jobid_t jobid);

/**
 * Structure for SNAPC v1.0.0 components.
 */
struct orte_snapc_base_component_1_0_0_t {
    /** MCA base component */
    mca_base_component_t snapc_version;
    /** MCA base data */
    mca_base_component_data_1_0_0_t snapc_data;

    /** Component Query for Selection Function */
    orte_snapc_base_component_query_1_0_0_fn_t snapc_query;
    
    /** Verbosity Level */
    int verbose;
    /** Output Handle for opal_output */
    int output_handle;
    /** Default Priority */
    int priority;
    
    /** Type of Coordinator */
    uint32_t coord_type;
};
typedef struct orte_snapc_base_component_1_0_0_t orte_snapc_base_component_1_0_0_t;
typedef struct orte_snapc_base_component_1_0_0_t orte_snapc_base_component_t;

/**
 * Structure for SNAPC v1.0.0 modules
 */
struct orte_snapc_base_module_1_0_0_t {
    /** Initialization Function */
    orte_snapc_base_module_init_fn_t           snapc_init;
    /** Finalization Function */
    orte_snapc_base_module_finalize_fn_t       snapc_finalize;
    /** Setup structures for a job */
    orte_snapc_base_setup_job_fn_t             setup_job;
    /** Release job */
    orte_snapc_base_release_job_fn_t           release_job;
};
typedef struct orte_snapc_base_module_1_0_0_t orte_snapc_base_module_1_0_0_t;
typedef struct orte_snapc_base_module_1_0_0_t orte_snapc_base_module_t;

ORTE_DECLSPEC extern orte_snapc_base_module_t orte_snapc;

/**
 * Macro for use in components that are of type SNAPC v1.0.0
 */
#define ORTE_SNAPC_BASE_VERSION_1_0_0 \
    /* SNAPC v1.0 is chained to MCA v1.0 */ \
    MCA_BASE_VERSION_1_0_0, \
    /* SNAPC v1.0 */ \
    "snapc", 1, 0, 0

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* ORTE_SNAPC_H */

