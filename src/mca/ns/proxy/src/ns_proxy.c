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
/** @file:
 *
 */

#include "orte_config.h"

#include <string.h>

#include "include/orte_constants.h"
#include "include/orte_types.h"
#include "mca/mca.h"
#include "dps/dps.h"
#include "mca/errmgr/errmgr.h"
#include "mca/rml/rml.h"

#include "ns_proxy.h"

/**
 * globals
 */

/*
 * functions
 */

int orte_ns_proxy_create_cellid(orte_cellid_t *cellid, char *site, char *resource)
{
    orte_buffer_t* cmd;
    orte_buffer_t* answer;
    orte_ns_cmd_flag_t command;
    size_t count;
    int rc;
    orte_ns_proxy_cell_info_t *cptr;

    /* set the default value of error */
    *cellid = ORTE_CELLID_MAX;
    
    /* check for errors */
    if (NULL == site || NULL == resource) {
        ORTE_ERROR_LOG(ORTE_ERR_BAD_PARAM);
        return ORTE_ERR_BAD_PARAM;
    }
    
    command = ORTE_NS_CREATE_CELLID_CMD;

    cmd = OBJ_NEW(orte_buffer_t);
    if (cmd == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, &command, 1, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        return rc;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, &site, 1, ORTE_STRING))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        return rc;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, &resource, 1, ORTE_STRING))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        return rc;
    }

    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, MCA_OOB_TAG_NS, 0)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(cmd);
        return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);

    answer = OBJ_NEW(orte_buffer_t);
    if(answer == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (0 > orte_rml.recv_buffer(orte_ns_my_replica, answer, ORTE_RML_TAG_NS)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, &command, &count, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        return rc;
    }
    
    if (ORTE_NS_CREATE_CELLID_CMD != command) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, cellid, &count, ORTE_CELLID))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        return rc;
    }
    OBJ_RELEASE(answer);
    
    /* store the info locally for later retrieval */
    cptr = OBJ_NEW(orte_ns_proxy_cell_info_t);
    if (NULL == cptr) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    
    cptr->cellid = *cellid;
    cptr->site = strdup(site);
    cptr->resource = strdup(resource);
    ompi_list_append(&orte_ns_proxy_cell_info_list, &cptr->item);
    
    return ORTE_SUCCESS;
}


int orte_ns_proxy_get_cell_info(orte_cellid_t cellid,
                                char **site, char **resource)
{
    ompi_list_item_t *item;
    orte_ns_proxy_cell_info_t *cell;
    
    *site = NULL;
    *resource = NULL;
    
    for (item = ompi_list_get_first(&orte_ns_proxy_cell_info_list);
         item != ompi_list_get_end(&orte_ns_proxy_cell_info_list);
         item = ompi_list_get_next(item)) {
        cell = (orte_ns_proxy_cell_info_t*)item;
        if (cellid == cell->cellid) {
            *site = strdup(cell->site);
            *resource = strdup(cell->resource);
            return ORTE_SUCCESS;
        }
    }
    return ORTE_ERR_NOT_FOUND;
}
                                
int orte_ns_proxy_create_jobid(orte_jobid_t *job)
{
    orte_buffer_t* cmd;
    orte_buffer_t* answer;
    orte_ns_cmd_flag_t command;
    size_t count;
    int rc;

    /* set default value */
    *job = ORTE_JOBID_MAX;
    
    if ((cmd = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    command = ORTE_NS_CREATE_JOBID_CMD;
    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&command, 1, ORTE_NS_CMD))) { /* got a problem */
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        return rc;
    }

    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, ORTE_RML_TAG_NS, 0)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(cmd);
        return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);

    if ((answer = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        OBJ_RELEASE(answer);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    if (0 > orte_rml.recv_buffer(orte_ns_my_replica, answer, ORTE_RML_TAG_NS)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, &command, &count, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        return rc;
    }
    
    if (ORTE_NS_CREATE_JOBID_CMD != command) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, job, &count, ORTE_JOBID))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        return rc;
    }
    
    OBJ_RELEASE(answer);
    return ORTE_SUCCESS;
}


int orte_ns_proxy_reserve_range(orte_jobid_t job, orte_vpid_t range, orte_vpid_t *starting_vpid)
{
    orte_buffer_t* cmd;
    orte_buffer_t* answer;
    orte_ns_cmd_flag_t command;
    size_t count;
    int rc;

    /* set default return value */
    *starting_vpid = ORTE_VPID_MAX;
    
    if ((cmd = OBJ_NEW(orte_buffer_t)) == NULL) {
       ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
       return ORTE_ERR_OUT_OF_RESOURCE;
    }

    command = ORTE_NS_RESERVE_RANGE_CMD;
    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&command, 1, ORTE_NS_CMD))) { /* got a problem */
       ORTE_ERROR_LOG(rc);
       OBJ_RELEASE(cmd);
       return rc;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&job, 1, ORTE_JOBID))) { /* got a problem */
       ORTE_ERROR_LOG(rc);
       OBJ_RELEASE(cmd);
       return rc;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&range, 1, ORTE_VPID))) { /* got a problem */
       ORTE_ERROR_LOG(rc);
       OBJ_RELEASE(cmd);
       return rc;
    }

    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, ORTE_RML_TAG_NS, 0)) {
       ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
       OBJ_RELEASE(cmd);
       return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);


    if ((answer = OBJ_NEW(orte_buffer_t)) == NULL) {
       ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
       return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (0 > orte_rml.recv_buffer(orte_ns_my_replica, answer, ORTE_RML_TAG_NS)) {
       ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
       OBJ_RELEASE(answer);
       return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if ((ORTE_SUCCESS != (rc = orte_dps.unpack(answer, &command, &count, ORTE_NS_CMD)))
    || (ORTE_NS_RESERVE_RANGE_CMD != command)) {
       ORTE_ERROR_LOG(rc);
       OBJ_RELEASE(answer);
       return rc;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, starting_vpid, &count, ORTE_VPID))) {
       ORTE_ERROR_LOG(rc);
       OBJ_RELEASE(answer);
       return rc;
    }
    OBJ_RELEASE(answer);
    return ORTE_SUCCESS;
}


int orte_ns_proxy_assign_rml_tag(orte_rml_tag_t *tag,
                                 char *name)
{
    orte_buffer_t* cmd;
    orte_buffer_t* answer;
    orte_ns_cmd_flag_t command;
    orte_ns_proxy_tagitem_t* tagitem;
    size_t count;
    int rc;

    OMPI_THREAD_LOCK(&orte_ns_proxy_mutex);

    if (NULL != name) {
        /* first, check to see if name is already on local list
         * if so, return tag
         */
        for (tagitem = (orte_ns_proxy_tagitem_t*)ompi_list_get_first(&orte_ns_proxy_taglist);
             tagitem != (orte_ns_proxy_tagitem_t*)ompi_list_get_end(&orte_ns_proxy_taglist);
             tagitem = (orte_ns_proxy_tagitem_t*)ompi_list_get_next(tagitem)) {
            if (0 == strcmp(name, tagitem->name)) { /* found name on list */
                *tag = tagitem->tag;
                OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
                return ORTE_SUCCESS;
            }
        }
    }   

    /* okay, not on local list - so go get one from tag server */
    command = ORTE_NS_ASSIGN_OOB_TAG_CMD;
    *tag = ORTE_RML_TAG_MAX;  /* set the default error value */
    
    if ((cmd = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&command, 1, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }

    if (NULL == name) {
        name = "NULL";
    }

    if (0 > (rc = orte_dps.pack(cmd, &name, 1, ORTE_STRING))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }
    
    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, ORTE_RML_TAG_NS, 0)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);

    if ((answer = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (0 > orte_rml.recv_buffer(orte_ns_my_replica, answer, ORTE_RML_TAG_NS)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, &command, &count, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }
    
    if (ORTE_NS_ASSIGN_OOB_TAG_CMD != command) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, tag, &count, ORTE_UINT32))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }
    
    OBJ_RELEASE(answer);
        
    /* add the new tag to the local list so we don't have to get it again */
    tagitem = OBJ_NEW(orte_ns_proxy_tagitem_t);
    if (NULL == tagitem) { /* out of memory */
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        *tag = ORTE_RML_TAG_MAX;
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    tagitem->tag = *tag;
    if (NULL != name) {
        tagitem->name = strdup(name);
    } else {
        tagitem->name = NULL;
    }
    ompi_list_append(&orte_ns_proxy_taglist, &tagitem->item);
    OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
    
    /* all done */
    return ORTE_SUCCESS;
}


int orte_ns_proxy_define_data_type(const char *name,
                                   orte_data_type_t *type)
{
    orte_buffer_t* cmd;
    orte_buffer_t* answer;
    orte_ns_cmd_flag_t command;
    orte_ns_proxy_dti_t *dti;
    size_t count;
    int rc=ORTE_SUCCESS;

    if (NULL == name || 0 < *type) {
        ORTE_ERROR_LOG(ORTE_ERR_BAD_PARAM);
        return ORTE_ERR_BAD_PARAM;
    }
    
    OMPI_THREAD_LOCK(&orte_ns_proxy_mutex);

    /* first, check to see if name is already on local list
     * if so, return id, ensure registered with dps
     */
    for (dti = (orte_ns_proxy_dti_t*)ompi_list_get_first(&orte_ns_proxy_dtlist);
         dti != (orte_ns_proxy_dti_t*)ompi_list_get_end(&orte_ns_proxy_dtlist);
         dti = (orte_ns_proxy_dti_t*)ompi_list_get_next(dti)) {
        if (0 == strcmp(name, dti->name)) { /* found name on list */
            *type = dti->id;
            OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
            return ORTE_SUCCESS;
        }
    }

    /* okay, not on local list - so go get one from tag server */
    command = ORTE_NS_DEFINE_DATA_TYPE_CMD;
    *type = ORTE_DPS_ID_MAX;  /* set the default error value */
    
    if ((cmd = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, (void*)&command, 1, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, &name, 1, ORTE_STRING))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }
    
    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, ORTE_RML_TAG_NS, 0)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(cmd);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);

    if ((answer = OBJ_NEW(orte_buffer_t)) == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (0 > orte_rml.recv_buffer(orte_ns_my_replica, answer, ORTE_RML_TAG_NS)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, &command, &count, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return rc;
    }
    
    if (ORTE_NS_ASSIGN_OOB_TAG_CMD != command) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_COMM_FAILURE;
    }

    count = 1;
    if (ORTE_SUCCESS != (rc = orte_dps.unpack(answer, type, &count, ORTE_DATA_TYPE))) {
        ORTE_ERROR_LOG(ORTE_ERR_UNPACK_FAILURE);
        OBJ_RELEASE(answer);
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_UNPACK_FAILURE;
    }
    OBJ_RELEASE(answer);
        
    /* add the new id to the local list so we don't have to get it again */
    dti = OBJ_NEW(orte_ns_proxy_dti_t);
    if (NULL == dti) { /* out of memory */
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        *type = ORTE_DPS_ID_MAX;
        OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    dti->id = *type;
    dti->name = strdup(name);
    ompi_list_append(&orte_ns_proxy_taglist, &dti->item);
    OMPI_THREAD_UNLOCK(&orte_ns_proxy_mutex);
    
    /* all done */
    return rc;
}

/*
 * Take advantage of the way the RML uses the process name as its index into
 * the RML communicator table. Because the RML needs a name right away, it will
 * automatically assign us one when it receives a message - and it communicates
 * that assignment back to us automatically. Thus, to get a name for ourselves,
 * all we have to do is send a message! No response from the replica is required.
 */
int orte_ns_proxy_create_my_name(void)
{
    orte_buffer_t* cmd;
    orte_ns_cmd_flag_t command;
    int rc;

    command = ORTE_NS_CREATE_MY_NAME_CMD;

    cmd = OBJ_NEW(orte_buffer_t);
    if (cmd == NULL) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }

    if (ORTE_SUCCESS != (rc = orte_dps.pack(cmd, &command, 1, ORTE_NS_CMD))) {
        ORTE_ERROR_LOG(rc);
        OBJ_RELEASE(cmd);
        return rc;
    }

    if (0 > orte_rml.send_buffer(orte_ns_my_replica, cmd, MCA_OOB_TAG_NS, 0)) {
        ORTE_ERROR_LOG(ORTE_ERR_COMM_FAILURE);
        OBJ_RELEASE(cmd);
        return ORTE_ERR_COMM_FAILURE;
    }
    OBJ_RELEASE(cmd);

    return ORTE_SUCCESS;
}

