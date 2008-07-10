/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
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

#include "orte_config.h"

#include <string.h>

#include "orte/util/show_help.h"

#include "opal/class/opal_hash_table.h"
#include "orte/mca/rml/rml.h"
#include "orte/mca/errmgr/errmgr.h"
#include "orte/runtime/orte_globals.h"

#include "orte/mca/iof/base/iof_base_header.h"
#include "orte/mca/iof/base/iof_base_fragment.h"
#include "iof_svc.h"
#include "iof_svc_proxy.h"
#include "iof_svc_pub.h"
#include "iof_svc_sub.h"



/**
 * Subscription onstructor/destructor 
 */

static void orte_iof_svc_sub_construct(orte_iof_svc_sub_t* sub)
{
    sub->sub_endpoint = NULL;
    sub->has_been_acked = true;
    sub->last_ack_forwarded = 0;
    OBJ_CONSTRUCT(&sub->sub_forward, opal_list_t);
}


static void orte_iof_svc_sub_destruct(orte_iof_svc_sub_t* sub)
{
    opal_list_item_t* item;
    if(sub->sub_endpoint != NULL)
        OBJ_RELEASE(sub->sub_endpoint);
    while(NULL != (item = opal_list_remove_first(&sub->sub_forward))) {
        OBJ_RELEASE(item);
    }
}


OBJ_CLASS_INSTANCE(
    orte_iof_svc_sub_t,
    opal_list_item_t,
    orte_iof_svc_sub_construct,
    orte_iof_svc_sub_destruct);

/**
 *  Create a subscription/forwarding entry.
 */

int orte_iof_svc_sub_create(
    const orte_process_name_t *origin_name,
    orte_ns_cmp_bitmask_t origin_mask,
    orte_iof_base_tag_t origin_tag,
    const orte_process_name_t *target_name,
    orte_ns_cmp_bitmask_t target_mask,
    orte_iof_base_tag_t target_tag)
{
    orte_iof_svc_sub_t* sub;
    opal_list_item_t* item;

    /* See if the subscription already exists */
    OPAL_THREAD_LOCK(&mca_iof_svc_component.svc_lock);
    for(item =  opal_list_get_first(&mca_iof_svc_component.svc_subscribed);
        item != opal_list_get_end(&mca_iof_svc_component.svc_subscribed);
        item =  opal_list_get_next(item)) {
        sub = (orte_iof_svc_sub_t*)item;
        if (sub->origin_mask == origin_mask &&
            OPAL_EQUAL == orte_util_compare_name_fields(sub->origin_mask,&sub->origin_name,origin_name) &&
            sub->origin_tag == origin_tag &&
            sub->target_mask == target_mask &&
            OPAL_EQUAL == orte_util_compare_name_fields(sub->target_mask,&sub->target_name,target_name) &&
            sub->target_tag == target_tag) {
                OPAL_THREAD_UNLOCK(&mca_iof_svc_component.svc_lock);
                return ORTE_SUCCESS;
        }
    }

    /* No, it does not -- create a new one */
    sub = OBJ_NEW(orte_iof_svc_sub_t);
    sub->origin_name = *origin_name;
    sub->origin_mask = origin_mask;
    sub->origin_tag = origin_tag;
    sub->target_name = *target_name;
    sub->target_mask = target_mask;
    sub->target_tag = target_tag;
    sub->sub_endpoint = orte_iof_base_endpoint_match(&sub->target_name, sub->target_mask, sub->target_tag);
    opal_output_verbose(1, orte_iof_base.iof_output, "created svc sub, origin %s tag %d / mask %x, target %s, tag %d / mask %x\n",
                ORTE_NAME_PRINT((orte_process_name_t*)origin_name), origin_tag, origin_mask,
                ORTE_NAME_PRINT((orte_process_name_t*)target_name), target_tag, target_mask);

    /* search through published endpoints for a match */
    for(item  = opal_list_get_first(&mca_iof_svc_component.svc_published);
        item != opal_list_get_end(&mca_iof_svc_component.svc_published);
        item  = opal_list_get_next(item)) {
        orte_iof_svc_pub_t* pub = (orte_iof_svc_pub_t*)item;
        if(orte_iof_svc_fwd_match(sub,pub)) {
            orte_iof_svc_fwd_create(sub,pub);
        }
    }

    opal_list_append(&mca_iof_svc_component.svc_subscribed, &sub->super);
    OPAL_THREAD_UNLOCK(&mca_iof_svc_component.svc_lock);
    return ORTE_SUCCESS;
}

/**
 * Release resources when the forwarding of an ACK has completed.
 */

static void ack_send_cb(
    int status,
    orte_process_name_t* peer,
    struct iovec* msg,
    int count,
    orte_rml_tag_t tag,
    void* cbdata)
{
    orte_iof_base_frag_t* frag = (orte_iof_base_frag_t*)cbdata;
    ORTE_IOF_BASE_FRAG_RETURN(frag);
    if(status < 0) {
        ORTE_ERROR_LOG(status);
    }
}

/**
 * We have received an ACK from one of the targets that we previously
 * forwarded a message to.  However, given the one-to-many nature of
 * IOF forwarding, we don't automatically forward that ACK on to the
 * origin of the original message.  Instead, we wait for *all* the
 * targets of the original message to reply with the apprpriate ACK,
 * and *then* we forward the ACK on to the original message's origin.
 *
 * In this way, the origin will only broadcast as fast as the slowest
 * target.
 *
 * Think of it this way: this function serves as a clearinghouse for
 * ACKs.  It will only push an ACK upstream to an origin when all the
 * corresponding targets have ACK'ed.
 */

void orte_iof_svc_sub_ack(
    const orte_process_name_t* peer,
    orte_iof_base_msg_header_t* hdr,
    bool do_close)
{
    opal_list_item_t *s_item;
    uint32_t seq_min = UINT32_MAX;
    uint32_t last_ack_forwarded = 0;
    bool has_been_acked = false;
    union {
        uint32_t uval;
        void *vval;
    } value;

    opal_output_verbose(1, orte_iof_base.iof_output, "orte_iof_svc_proxy_ack");
    if (do_close) {
        opal_output_verbose(1, orte_iof_base.iof_output, "CLOSE ACK!\n");
    }

    /* for each of the subscriptions that match the origin of the ACK:
     * (1) find all forwarding entries that match the origin of the ACK
     * (2) update their sequence number
     * (3) find the minimum sequence number across all endpoints 
    */
     
    OPAL_THREAD_LOCK(&mca_iof_svc_component.svc_lock);
    for(s_item =  opal_list_get_first(&mca_iof_svc_component.svc_subscribed);
        s_item != opal_list_get_end(&mca_iof_svc_component.svc_subscribed);
        s_item =  opal_list_get_next(s_item)) {

        orte_iof_svc_sub_t* sub = (orte_iof_svc_sub_t*)s_item;
        opal_list_item_t *f_item;

        opal_output_verbose(1, orte_iof_base.iof_output, "ack: checking sub origin %s tag %d / mask %x, target %s, tag %d / mask %x\n",
                    ORTE_NAME_PRINT(&sub->origin_name), sub->origin_tag, sub->origin_mask,
                    ORTE_NAME_PRINT(&sub->target_name), sub->target_tag, sub->target_mask);

        /* If the subscription origin/tag doesn't match the ACK
           origin/tag, skip it */
        if (OPAL_EQUAL != orte_util_compare_name_fields(sub->origin_mask,
                                                        &sub->origin_name, 
                                                        &hdr->msg_origin) ||
            sub->origin_tag != hdr->msg_tag) {
            continue;
        }

        /* We match, so keep a running tally of whether the ACK has
           been forwarded or not, and if so, how many bytes have been
           ACK'ed. */
        has_been_acked |= sub->has_been_acked;
        if (sub->has_been_acked) {
            if (last_ack_forwarded > sub->last_ack_forwarded) {
                last_ack_forwarded = sub->last_ack_forwarded;
            }
        }
        opal_output_verbose(1, orte_iof_base.iof_output,
                    "ack: has_beed_acked: %d, last forwarded %d",
                    has_been_acked, last_ack_forwarded);

        /* If the subscription has a local endpoint and the ACK is
           coming from this process, then update the seq_min
           calculation */
        if (NULL != sub->sub_endpoint &&
            OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,
                                                        ORTE_PROC_MY_NAME,
                                                        peer)) {
            if (do_close) {
                /* JMS what to do here?  Need to set sub->sub_endpoint
                   to NULL.  Have similar leak for do_close for
                   streams.  See ticket #1048. */
                sub->sub_endpoint = NULL;
                opal_output_verbose(1, orte_iof_base.iof_output,
                            "ack: CLOSED local ack to %u", value.uval);
            } else {
                value.uval = hdr->msg_seq + hdr->msg_len;
                opal_output_verbose(1, orte_iof_base.iof_output,
                            "ack: local ack to %u", value.uval);
                if (value.uval < seq_min) {
                    seq_min = value.uval;
                }
            }
        }

        /* Find the minimum amount ack'ed by all the origins (or,
           technically speaking, ack'ed by their proxies on their
           behalf) */
        for(f_item =  opal_list_get_first(&sub->sub_forward);
            f_item != opal_list_get_end(&sub->sub_forward);
            f_item =  opal_list_get_next(f_item)) {
            orte_iof_svc_fwd_t* fwd = (orte_iof_svc_fwd_t*)f_item;
            orte_iof_svc_pub_t* pub = fwd->fwd_pub;
            bool value_set = true;

            opal_output_verbose(1, orte_iof_base.iof_output, "ack: checking fwd %s tag %d / mask %x\n",
                        ORTE_NAME_PRINT(&pub->pub_name), pub->pub_tag, pub->pub_mask);

            /* If the publication origin or publication proxy matches
               the ACK'ing proxy, save the ACK'ed byte count for this
               *origin* (not the proxy). */
            if (OPAL_EQUAL == orte_util_compare_name_fields(pub->pub_mask,&pub->pub_name,peer) ||
                OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,&pub->pub_proxy,peer)) {
                opal_output_verbose(1, orte_iof_base.iof_output,
                            "ack: found matching pub");
                /* If we're closing, then remove this proc from
                   the table -- we won't be using its value to
                   calculate seq_min anymore.  Otherwise, put its
                   updated value in the table. */
                if (do_close) {
                    opal_hash_table_remove_value_uint64(&fwd->fwd_seq_hash,
                                        orte_util_hash_name(&hdr->msg_origin));
                    value_set = false;
                } else {
                    value.uval = hdr->msg_seq + hdr->msg_len;
                    opal_hash_table_set_value_uint64(&fwd->fwd_seq_hash,
                                        orte_util_hash_name(&hdr->msg_origin), &value.vval);
                }
            } 
            /* Otherwise, if the publication origin and publication
               proxy do not match the ACK'ing proxy, then lookup
               whatever byte count was previously ACK'ed for the origin
               and use that to compute the minimum byte count ACK'ed
               so far.

               As such, even though the logic is confusing, at the end
               of this loop, seq_min will have the minimum number of
               bytes ACK'ed across all the forwards on this
               subscription. */
            else {
                opal_hash_table_get_value_uint64(&fwd->fwd_seq_hash,
                        orte_util_hash_name(&hdr->msg_origin), (void**)&value.vval);
            }

            /* If we got a valid value, update the seq_min calculation */
            if (value_set && value.uval < seq_min) {
                seq_min = value.uval;
            }
        }
    }
    OPAL_THREAD_UNLOCK(&mca_iof_svc_component.svc_lock);

    /* If nothing changed ACK-wise (including the situation where we
       are closing and there's no subscriber left to ACK), then we're
       done.  NOTE: this isn't technically right; if there's no
       subscribers left, we should do some more cleanup than this.
       But that's coming in ticket #1049 and/or #1051. */

    if (seq_min == UINT32_MAX) {
        return;
    }

    /* If everyone has ACK'ed, then push the ACK up to the original
       message's proxy */
    if (seq_min == hdr->msg_seq+hdr->msg_len) {
        /* If the original message was initiated from this process,
           then the ACK delivery is local. */
        if (OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,
                                                        ORTE_PROC_MY_NAME,
                                                        &hdr->msg_origin) ||
            OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,
                                                        ORTE_PROC_MY_NAME,
                                                        &hdr->msg_proxy)) {
            orte_iof_base_endpoint_t* endpoint;
            endpoint = orte_iof_base_endpoint_match(&hdr->msg_origin, 
                                                    ORTE_NS_CMP_ALL, 
                                                    hdr->msg_tag);
            if (NULL != endpoint) {
                opal_output_verbose(1, orte_iof_base.iof_output,
                            "ack: forwarding ack locally: %u", seq_min);
                orte_iof_base_endpoint_ack(endpoint, seq_min);
                OBJ_RELEASE(endpoint);
            }
        }
        /* Otherwise, the original message was initiated in another
           process, and we need to forward the ACK to it. */
        else {
            orte_iof_base_frag_t* frag;
            int rc;
    
            ORTE_IOF_BASE_FRAG_ALLOC(frag,rc);
            if(NULL == frag) {
                ORTE_ERROR_LOG(rc);
                return;
            }

            frag->frag_hdr.hdr_msg = *hdr;
            frag->frag_iov[0].iov_base = (IOVBASE_TYPE*)&frag->frag_hdr;
            frag->frag_iov[0].iov_len = sizeof(frag->frag_hdr);
            ORTE_IOF_BASE_HDR_MSG_HTON(frag->frag_hdr.hdr_msg);

            opal_output_verbose(1, orte_iof_base.iof_output,
                        "ack: forwarding ack remotely: %u", seq_min);
            rc = orte_rml.send_nb(
                &hdr->msg_proxy,
                frag->frag_iov,
                1,
                ORTE_RML_TAG_IOF_SVC,
                0,
                ack_send_cb,
                frag);
            if(rc < 0) {
                ORTE_ERROR_LOG(rc);
            }
        }
    }
}

/**
 *  Delete all matching subscriptions.
 */

int orte_iof_svc_sub_delete(
    const orte_process_name_t *origin_name,
    orte_ns_cmp_bitmask_t origin_mask,
    orte_iof_base_tag_t origin_tag,
    const orte_process_name_t *target_name,
    orte_ns_cmp_bitmask_t target_mask,
    orte_iof_base_tag_t target_tag)
{
    opal_list_item_t *item;
    OPAL_THREAD_LOCK(&mca_iof_svc_component.svc_lock);
    item =  opal_list_get_first(&mca_iof_svc_component.svc_subscribed);
    while(item != opal_list_get_end(&mca_iof_svc_component.svc_subscribed)) {
        opal_list_item_t* next =  opal_list_get_next(item);
        orte_iof_svc_sub_t* sub = (orte_iof_svc_sub_t*)item;
        if (sub->origin_mask == origin_mask &&
            OPAL_EQUAL == orte_util_compare_name_fields(sub->origin_mask,&sub->origin_name,origin_name) &&
            sub->origin_tag == origin_tag &&
            sub->target_mask == target_mask &&
            OPAL_EQUAL == orte_util_compare_name_fields(sub->target_mask,&sub->target_name,target_name) &&
            sub->target_tag == target_tag) {
            opal_list_remove_item(&mca_iof_svc_component.svc_subscribed, item);
            OBJ_RELEASE(item);
        }
        item = next;
    }
    OPAL_THREAD_UNLOCK(&mca_iof_svc_component.svc_lock);
    return ORTE_SUCCESS;
}


int orte_iof_svc_sub_delete_all(
    const orte_process_name_t *name)
{
    opal_list_item_t *item;
    OPAL_THREAD_LOCK(&mca_iof_svc_component.svc_lock);
    item =  opal_list_get_first(&mca_iof_svc_component.svc_subscribed);
    while(item != opal_list_get_end(&mca_iof_svc_component.svc_subscribed)) {
        opal_list_item_t* next =  opal_list_get_next(item);
        orte_iof_svc_sub_t* sub = (orte_iof_svc_sub_t*)item;
        if ((sub->origin_mask == ORTE_NS_CMP_ALL &&
             OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,&sub->origin_name,name)) ||
            (sub->target_mask == ORTE_NS_CMP_ALL &&
             OPAL_EQUAL == orte_util_compare_name_fields(ORTE_NS_CMP_ALL,&sub->target_name,name))) {
            opal_list_remove_item(&mca_iof_svc_component.svc_subscribed, item);
            OBJ_RELEASE(item);
        }
        item = next;
    }
    OPAL_THREAD_UNLOCK(&mca_iof_svc_component.svc_lock);
    return ORTE_SUCCESS;
}



/*
 * Callback on send completion. Release send resources (fragment).
 */

static void orte_iof_svc_sub_send_cb(
    int status,
    orte_process_name_t* peer,
    struct iovec* msg,
    int count,
    orte_rml_tag_t tag,
    void* cbdata)
{
    orte_iof_base_frag_t* frag = (orte_iof_base_frag_t*)cbdata;
    ORTE_IOF_BASE_FRAG_RETURN(frag);
    if(status < 0) {
        ORTE_ERROR_LOG(status);
    }
}

/**
 *  Check for matching endpoints that have been published to the
 *  server. Forward data out each matching endpoint.
 */

int orte_iof_svc_sub_forward(
    orte_iof_svc_sub_t* sub,
    const orte_process_name_t* src,
    orte_iof_base_msg_header_t* hdr,
    const unsigned char* data,
    bool *forward)
{
    opal_list_item_t* item;
    for(item  = opal_list_get_first(&sub->sub_forward);
        item != opal_list_get_end(&sub->sub_forward);
        item =  opal_list_get_next(item)) {
        orte_iof_svc_fwd_t* fwd = (orte_iof_svc_fwd_t*)item;
        orte_iof_svc_pub_t* pub = fwd->fwd_pub;
        int rc;

        if (NULL != pub->pub_endpoint) {
            /* Here's a fun case: if the HNP launches VPID 0 on the
               same node as itself, then the HNP's stdin will both be
               published *and* subscribed.  So when something appears
               on the HNP stdin, it gets RML sent from the HNP to
               itself (yeah, I know, it's weird -- but there were
               reasons for doing that ;-) ), ends up going through:

               - orte_iof_svc_proxy_recv (RML callback function)
               - orte_iov_svc_proxy_msg (dispatch to handle incoming msg)
               - orte_iof_svc_sub_forward (this function, to forward
                 the message to its destination(s))

               STDIN from the HNP to a local VPID 0 is the only case
               where the message will find an endpoint with both a pub
               and sub on it, so we want to be sure to forward it only
               *once*.  Outside of this loop, we'll forward it to
               sub->sub_endpoint.  So if the pub->pub_endpoint is the
               same as the sub->sub_endpoint, then skip it here --
               we'll forward it after this loop.  Without this check,
               we'd forward it twice (resulting in
               https://svn.open-mpi.org/trac/ompi/ticket/1135). */
            if (pub->pub_endpoint != sub->sub_endpoint) {
                opal_output_verbose(1, orte_iof_base.iof_output, "sub_forward: forwarding to pub local endpoint");
                rc = orte_iof_base_endpoint_forward(pub->pub_endpoint,src,hdr,data);
            }
        } else {
            /* forward */
            orte_iof_base_frag_t* frag;
            opal_output_verbose(1, orte_iof_base.iof_output, "sub_forward: forwarding to pub stream / remote endpoint");
            ORTE_IOF_BASE_FRAG_ALLOC(frag,rc);
            frag->frag_hdr.hdr_msg = *hdr;
            frag->frag_len = frag->frag_hdr.hdr_msg.msg_len;
            frag->frag_iov[0].iov_base = (IOVBASE_TYPE*)&frag->frag_hdr;
            frag->frag_iov[0].iov_len = sizeof(frag->frag_hdr);
            frag->frag_iov[1].iov_base = (IOVBASE_TYPE*)frag->frag_data;
            frag->frag_iov[1].iov_len = frag->frag_len;
            memcpy(frag->frag_data, data, frag->frag_len);
            ORTE_IOF_BASE_HDR_MSG_HTON(frag->frag_hdr.hdr_msg);
            rc = orte_rml.send_nb(
                &pub->pub_proxy,
                frag->frag_iov,
                2,
                ORTE_RML_TAG_IOF_SVC,
                0,
                orte_iof_svc_sub_send_cb,
                frag);
        }
        if (ORTE_SUCCESS != rc) {
            return rc;
        }
        *forward = true;
    }
    if (NULL != sub->sub_endpoint) {
        opal_output_verbose(1, orte_iof_base.iof_output, "sub_forward: forwarding to sub local endpoint");
        *forward = true;
        return orte_iof_base_endpoint_forward(sub->sub_endpoint,src,hdr,data); 
    }
    return ORTE_SUCCESS;
}


/**
 * I/O Forwarding entry - relates a published endpoint to
 * a subscription.
 */

static void orte_iof_svc_fwd_construct(orte_iof_svc_fwd_t* fwd)
{
    fwd->fwd_pub = NULL;
    OBJ_CONSTRUCT(&fwd->fwd_seq_hash, opal_hash_table_t);
    opal_hash_table_init(&fwd->fwd_seq_hash, 256);
}

static void orte_iof_svc_fwd_destruct(orte_iof_svc_fwd_t* fwd)
{
    if(NULL != fwd->fwd_pub) {
        OBJ_RELEASE(fwd->fwd_pub);
    }
    OBJ_DESTRUCT(&fwd->fwd_seq_hash);
}


OBJ_CLASS_INSTANCE(
    orte_iof_svc_fwd_t,
    opal_list_item_t,
    orte_iof_svc_fwd_construct,
    orte_iof_svc_fwd_destruct);

/**
 *  Does the published endpoint match the destination specified
 *  in the subscription?
 */

bool orte_iof_svc_fwd_match(
    orte_iof_svc_sub_t* sub,
    orte_iof_svc_pub_t* pub)
{
    if (OPAL_EQUAL == orte_util_compare_name_fields(sub->target_mask,&sub->target_name,&pub->pub_name) &&
        sub->origin_tag == pub->pub_tag) {
        return true;
    } else {
        return false;
    }
}


/**
 *  Create a forwarding entry
 */

int orte_iof_svc_fwd_create(
    orte_iof_svc_sub_t* sub,
    orte_iof_svc_pub_t* pub)
{
    orte_iof_svc_fwd_t* fwd = OBJ_NEW(orte_iof_svc_fwd_t);
    if(NULL == fwd) {
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    OBJ_RETAIN(pub);
    fwd->fwd_pub = pub;
    opal_output_verbose(1, orte_iof_base.iof_output, "created svc forward, sub origin %s, tag %d / mask %x, sub target %s, tag %d / mask %x :::: pub name %s, tag %d / mask %x\n",
                ORTE_NAME_PRINT(&sub->origin_name), sub->origin_tag,
                sub->origin_mask,
                ORTE_NAME_PRINT(&sub->target_name), sub->target_tag,
                sub->target_mask,
                ORTE_NAME_PRINT(&pub->pub_name), pub->pub_tag, pub->pub_mask);
    opal_list_append(&sub->sub_forward, &fwd->super);
    return ORTE_SUCCESS;
}


/**
 *  Remove any forwarding entries that match the
 *  published endpoint.
 */

int orte_iof_svc_fwd_delete(
    orte_iof_svc_sub_t* sub,
    orte_iof_svc_pub_t* pub)
{
    opal_list_item_t* item;
    for(item =  opal_list_get_first(&sub->sub_forward);
        item != opal_list_get_end(&sub->sub_forward);
        item =  opal_list_get_next(item)) {
        orte_iof_svc_fwd_t* fwd = (orte_iof_svc_fwd_t*)item;
        if(fwd->fwd_pub == pub) {
            opal_list_remove_item(&sub->sub_forward,item);
            OBJ_RELEASE(fwd);
            return ORTE_SUCCESS;
        }
    }
    return ORTE_ERR_NOT_FOUND;
}


