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


#include "btl_mvapi_frag.h" 
#include "mca/common/vapi/vapi_mem_reg.h"
#include "mca/mpool/mvapi/mpool_mvapi.h" 



static void mca_btl_mvapi_frag_common_constructor( mca_btl_mvapi_frag_t* frag) 
{
    mca_mpool_mvapi_registration_t* mem_hndl = (mca_mpool_mvapi_registration_t*) frag->base.super.user_data; 
    frag->hdr = (mca_btl_mvapi_header_t*) (frag+1);    /* initialize the btl header to point to start at end of frag */ 
#if 0   
    mod = (unsigned long) frag->hdr % MCA_BTL_IB_FRAG_ALIGN; 
    
    if(mod != 0) {
        frag->hdr = (mca_btl_mvapi_header_t*) ((unsigned char*) frag->hdr + (MCA_BTL_IB_FRAG_ALIGN - mod));
    }
#endif 
    
    frag->segment.seg_addr.pval = ((unsigned char* )frag->hdr) + sizeof(mca_btl_mvapi_header_t);  /* init the segment address to start after the btl header */ 
    
#if 0 
    mod = (frag->segment.seg_addr.lval) % MCA_BTL_IB_FRAG_ALIGN; 
    if(mod != 0) {
        frag->segment.seg_addr.lval += (MCA_BTL_IB_FRAG_ALIGN - mod);
    }
#endif 
    
    /* frag->mem_hndl = mem_hndl->hndl;  */
    frag->segment.seg_len = frag->size;
    frag->segment.seg_key.key32[0] = (uint32_t) mem_hndl->l_key; 
    frag->sg_entry.lkey = mem_hndl->l_key; 
    frag->sg_entry.addr = (VAPI_virt_addr_t) (MT_virt_addr_t) frag->hdr; 
    frag->base.des_flags = 0; 

}

 
static void mca_btl_mvapi_send_frag_common_constructor(mca_btl_mvapi_frag_t* frag) 
{ 
    
    mca_btl_mvapi_frag_common_constructor(frag); 
    frag->base.des_src = &frag->segment;
    frag->base.des_src_cnt = 1;
    frag->base.des_dst = NULL;
    frag->base.des_dst_cnt = 0;
    
    frag->sr_desc.comp_type = VAPI_SIGNALED; 
    frag->sr_desc.opcode = VAPI_SEND; 
    frag->sr_desc.remote_qkey = 0; 
    frag->sr_desc.sg_lst_len = 1; 
    frag->sr_desc.sg_lst_p = &frag->sg_entry; 
    frag->sr_desc.id = (VAPI_virt_addr_t) (MT_virt_addr_t) frag; 
    
}

static void mca_btl_mvapi_recv_frag_common_constructor(mca_btl_mvapi_frag_t* frag) 
{ 
    
    mca_btl_mvapi_frag_common_constructor(frag); 
    frag->base.des_dst = &frag->segment;
    frag->base.des_dst_cnt = 1;
    frag->base.des_src = NULL;
    frag->base.des_src_cnt = 0;
    
    frag->rr_desc.comp_type = VAPI_SIGNALED; 
    frag->rr_desc.opcode = VAPI_RECEIVE; 
    frag->rr_desc.sg_lst_len = 1; 
    frag->rr_desc.sg_lst_p = &frag->sg_entry; 
    frag->rr_desc.id = (VAPI_virt_addr_t) (MT_virt_addr_t) frag; 
    
   
}



static void mca_btl_mvapi_send_frag_eager_constructor(mca_btl_mvapi_frag_t* frag) 
{ 
    
    frag->size = mca_btl_mvapi_component.eager_limit;  
    mca_btl_mvapi_send_frag_common_constructor(frag); 
}


static void mca_btl_mvapi_send_frag_max_constructor(mca_btl_mvapi_frag_t* frag) 
{ 
    
    frag->size = mca_btl_mvapi_component.max_send_size; 
    mca_btl_mvapi_send_frag_common_constructor(frag); 
}

static void mca_btl_mvapi_recv_frag_max_constructor(mca_btl_mvapi_frag_t* frag) 
{
    frag->size = mca_btl_mvapi_component.max_send_size; 
    mca_btl_mvapi_recv_frag_common_constructor(frag); 
    
}


static void mca_btl_mvapi_recv_frag_eager_constructor(mca_btl_mvapi_frag_t* frag) 
{
    frag->size = mca_btl_mvapi_component.eager_limit; 
    mca_btl_mvapi_recv_frag_common_constructor(frag); 
    
}

static void mca_btl_mvapi_send_frag_frag_constructor(mca_btl_mvapi_frag_t* frag) 
{ 
    
    frag->size = 0; 
    mca_btl_mvapi_send_frag_common_constructor(frag); 
}


OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_frag_t, 
                   mca_btl_base_descriptor_t, 
                   NULL, 
                   NULL); 

OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_send_frag_eager_t, 
                   mca_btl_base_descriptor_t, 
                   mca_btl_mvapi_send_frag_eager_constructor, 
                   NULL); 


OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_send_frag_max_t, 
                   mca_btl_base_descriptor_t, 
                   mca_btl_mvapi_send_frag_max_constructor, 
                   NULL); 

OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_send_frag_frag_t, 
                   mca_btl_base_descriptor_t, 
                   mca_btl_mvapi_send_frag_frag_constructor, 
                   NULL); 

OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_recv_frag_eager_t, 
                   mca_btl_base_descriptor_t, 
                   mca_btl_mvapi_recv_frag_eager_constructor, 
                   NULL); 


OBJ_CLASS_INSTANCE(
                   mca_btl_mvapi_recv_frag_max_t, 
                   mca_btl_base_descriptor_t, 
                   mca_btl_mvapi_recv_frag_max_constructor, 
                   NULL); 


