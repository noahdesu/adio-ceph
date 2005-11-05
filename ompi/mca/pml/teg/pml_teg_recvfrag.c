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
 */

/**
 * @file
 */

#include "ompi_config.h"

#include "mca/pml/pml.h"
#include "pml_teg_recvfrag.h"
#include "pml_teg_proc.h"


OMPI_DECLSPEC extern opal_class_t mca_ptl_base_recv_frag_t_class;


/**
 * Called by the PTL to match attempt a match for new fragments.
 * 
 * @param ptl (IN)      The PTL pointer
 * @param frag (IN)     Receive fragment descriptor.
 * @param header (IN)   Header corresponding to the receive fragment.
 * @return              OMPI_SUCCESS or error status on failure.
 */
bool mca_pml_teg_recv_frag_match(
    mca_ptl_base_module_t* ptl,
    mca_ptl_base_recv_frag_t* frag, 
    mca_ptl_base_match_header_t* header)
{
    bool matched;
    bool matches = false;
    opal_list_t matched_frags;
    if((matched = mca_ptl_base_match(header, frag, &matched_frags, &matches)) == false) {
        frag = (matches ? (mca_ptl_base_recv_frag_t*)opal_list_remove_first(&matched_frags) : NULL);
    }

    while(NULL != frag) {
        mca_ptl_base_module_t* ptl = frag->frag_base.frag_owner;
        mca_ptl_base_recv_request_t *request = frag->frag_request;
        mca_ptl_base_match_header_t *header = &frag->frag_base.frag_header.hdr_match;

        /*
         * Initialize request status.
         */
        /* TODO request->req_recv.req_bytes_packed = header->hdr_msg_length; */
        request->req_recv.req_base.req_ompi.req_status.MPI_SOURCE = header->hdr_src;
        request->req_recv.req_base.req_ompi.req_status.MPI_TAG = header->hdr_tag;

        /*
         * If probe - signal request is complete - but don't notify PTL
         */
        if(request->req_recv.req_base.req_type == MCA_PML_REQUEST_PROBE) {

             ptl->ptl_recv_progress( ptl, 
                                     request,
                                     header->hdr_msg_length,
                                     header->hdr_msg_length );
             matched = mca_pml_teg_recv_frag_match( ptl, frag, header );

        } else {

            /* if required - setup pointer to ptls peer */
            if (NULL == frag->frag_base.frag_peer) {
                frag->frag_base.frag_peer = mca_pml_teg_proc_lookup_remote_peer(
                    request->req_recv.req_base.req_comm,header->hdr_src,ptl);
            }

            MCA_PML_TEG_RECV_MATCHED( ptl, frag );
        };

        /* process any additional fragments that arrived out of order */
        frag = (matches ? (mca_ptl_base_recv_frag_t*)opal_list_remove_first(&matched_frags) : NULL);
    };
    return matched;
}


