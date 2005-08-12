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

#include "ompi_config.h"

#include <stdlib.h>
#include <string.h>

#include "class/ompi_bitmap.h"
#include "mca/bml/bml.h"
#include "mca/btl/btl.h"
#include "mca/btl/base/base.h"
#include "mca/bml/base/bml_base_endpoint.h" 
#include "mca/bml/base/bml_base_btl.h" 
#include "bml_r2.h"
#include "class/orte_proc_table.h" 

extern mca_bml_base_component_t mca_bml_r2_component; 

mca_bml_r2_module_t mca_bml_r2 = {
    {
        &mca_bml_r2_component, 
        0, /* eager limit */ 
        0, /* min send size */ 
        0, /* max send size */ 
        0, /* min rdma size */ 
        0, /* max rdma size */ 
        mca_bml_r2_add_procs,
        mca_bml_r2_del_procs,
        mca_bml_r2_register, 
        mca_bml_r2_finalize, 
        mca_bml_r2_progress
    }
    
};


static int btl_exclusivity_compare(const void* arg1, const void* arg2)
{
    mca_btl_base_module_t* btl1 = *(struct mca_btl_base_module_t**)arg1;
    mca_btl_base_module_t* btl2 = *(struct mca_btl_base_module_t**)arg2;
    if( btl1->btl_exclusivity > btl2->btl_exclusivity ) {
        return -1;
    } else if (btl1->btl_exclusivity == btl2->btl_exclusivity ) {
        return 0;
    } else {
        return 1;
    }
}

void mca_bml_r2_recv_callback(
                              mca_btl_base_module_t* btl, 
                              mca_btl_base_tag_t tag, 
                              mca_btl_base_descriptor_t* desc, 
                              void* cbdata
                              ){ 

    /* just pass it up the stack.. */ 
    mca_bml_r2.r2_reg[tag](btl,
                           tag, 
                           desc, 
                           cbdata); 
    
}


int mca_bml_r2_progress( void ) { 
    size_t i;
    int count = 0;

    /*
     * Progress each of the PTL modules
     */
    for(i=0; i<mca_bml_r2.num_btl_progress; i++) {
        int rc = mca_bml_r2.btl_progress[i]();
        if(rc > 0) {
            count += rc;
        }
    }
    return count;
    
}


static int mca_bml_r2_add_btls( void )
{
    /* build an array of r2s and r2 modules */
    opal_list_t* btls = &mca_btl_base_modules_initialized;
    mca_btl_base_selected_module_t* selected_btl;
    size_t num_btls = opal_list_get_size(btls);

    mca_bml_r2.num_btl_modules = 0;
    mca_bml_r2.num_btl_progress = 0;
   
    mca_bml_r2.btl_modules = (mca_btl_base_module_t **)malloc(sizeof(mca_btl_base_module_t*) * num_btls);
    mca_bml_r2.btl_progress = (mca_btl_base_component_progress_fn_t*)malloc(sizeof(mca_btl_base_component_progress_fn_t) * num_btls);
   

    if (NULL == mca_bml_r2.btl_modules || 
        NULL == mca_bml_r2.btl_progress) {
        return OMPI_ERR_OUT_OF_RESOURCE;
    }

    for(selected_btl =  (mca_btl_base_selected_module_t*)opal_list_get_first(btls);
        selected_btl != (mca_btl_base_selected_module_t*)opal_list_get_end(btls);
        selected_btl =  (mca_btl_base_selected_module_t*)opal_list_get_next(selected_btl)) {
        mca_btl_base_module_t *btl = selected_btl->btl_module;
        mca_bml_r2.btl_modules[mca_bml_r2.num_btl_modules++] = btl;
    }


    /* sort r2 list by exclusivity */
    qsort(mca_bml_r2.btl_modules, 
          mca_bml_r2.num_btl_modules, 
          sizeof(struct mca_btl_base_module_t*), 
          btl_exclusivity_compare);
    return OMPI_SUCCESS;
}

/*
 *   For each proc setup a datastructure that indicates the PTLs
 *   that can be used to reach the destination.
 *
 */

int mca_bml_r2_add_procs(
                         size_t nprocs, 
                         struct ompi_proc_t** procs, 
                         struct mca_bml_base_endpoint_t** bml_endpoints, 
                         struct ompi_bitmap_t* reachable
                         )
{
    size_t p;
    int rc;
    size_t p_index;
    struct mca_bml_base_btl_t** bml_btls = NULL; 
    struct mca_btl_base_endpoint_t ** btl_endpoints = NULL;  
    if(nprocs == 0)
        return OMPI_SUCCESS;

    if(OMPI_SUCCESS != (rc = mca_bml_r2_add_btls()) )
        return rc;
    

    /* attempt to add all procs to each r2 */
    btl_endpoints = (struct mca_btl_base_endpoint_t **) malloc(nprocs * sizeof(struct mca_btl_base_endpoint_t*)); 
    bml_endpoints = (struct mca_bml_base_endpoint_t **)malloc(nprocs * sizeof(struct mca_bml_base_endpoint_t*));
    bml_btls = (struct mca_bml_base_btl_t **) malloc(nprocs * sizeof(struct mca_bml_base_btl_t*)); 

    for(p_index = 0; p_index < mca_bml_r2.num_btl_modules; p_index++) {
        mca_btl_base_module_t* btl = mca_bml_r2.btl_modules[p_index];
        int btl_inuse = 0;
        
        /* if the r2 can reach the destination proc it sets the
         * corresponding bit (proc index) in the reachable bitmap
         * and can return addressing information for each proc
         * that is passed back to the r2 on data transfer calls
         */
        ompi_bitmap_clear_all_bits(reachable);
        memset(bml_endpoints, 0, nprocs * sizeof(struct mca_bml_base_endpoint_t*));
        memset(btl_endpoints, 0, nprocs *sizeof(struct mca_btl_base_endpoint_t*)); 
        memset(bml_btls, 0, nprocs * sizeof(struct mca_bml_base_btl_t*)); 


        rc = btl->btl_add_procs(btl, nprocs, procs, btl_endpoints, reachable);
        if(OMPI_SUCCESS != rc) {
            free(btl_endpoints);
            return rc;
        }

        /* for each proc that is reachable - add the endpoint to the bml_endpoints array(s) */
        for(p=0; p<nprocs; p++) {
            if(ompi_bitmap_is_set_bit(reachable, p)) {
                ompi_proc_t *proc = procs[p]; 
/*                 mca_bml_base_endpoint_t* bml_endpoint = (mca_bml_base_endpoint_t*) proc->proc_pml; */
                mca_bml_base_endpoint_t * bml_endpoint; 
                mca_bml_base_btl_t* bml_btl; 
                size_t size;
                /*                 proc = (ompi_proc_t*) orte_hash_table_get_proc( */
/*                                                                &mca_bml_r2.procs,  */
/*                                                                &proc->proc_name */
/*                                                                );  */
                if(NULL != proc && NULL != proc->proc_pml) { 
                    bml_endpoints[p] =(mca_bml_base_endpoint_t*)  proc->proc_pml; 
                    continue; 
                }
                /* this btl can be used */
                btl_inuse++;

                /* allocate bml specific proc data */
                bml_endpoint = OBJ_NEW(mca_bml_base_endpoint_t);
                if (NULL == bml_endpoint) {
                    opal_output(0, "mca_bml_r2_add_procs: unable to allocate resources");
                    free(btl_endpoints);
                    return OMPI_ERR_OUT_OF_RESOURCE;
                }
                
                /* preallocate space in array for max number of r2s */
                mca_bml_base_btl_array_reserve(&bml_endpoint->btl_eager, mca_bml_r2.num_btl_modules);
                mca_bml_base_btl_array_reserve(&bml_endpoint->btl_send,  mca_bml_r2.num_btl_modules);
                mca_bml_base_btl_array_reserve(&bml_endpoint->btl_rdma,  mca_bml_r2.num_btl_modules);
                bml_endpoint->btl_proc =   proc;
                
                /*proc->proc_pml = (struct mca_pml_proc_t*) bml_endpoint;*/ 
                

                /* dont allow an additional PTL with a lower exclusivity ranking */
                size = mca_bml_base_btl_array_get_size(&bml_endpoint->btl_send);
                if(size > 0) {
                    bml_btl = mca_bml_base_btl_array_get_index(&bml_endpoint->btl_send, size-1);
                    /* skip this btl if the exclusivity is less than the previous */
                    if(bml_btl->btl->btl_exclusivity > btl->btl_exclusivity) {
                        if(btl_endpoints[p] != NULL) {
                            btl->btl_del_procs(btl, 1, &proc, &btl_endpoints[p]);
                        }
                        continue;
                    }
                }
                
                
                /* cache the endpoint on the proc */
                bml_btl = mca_bml_base_btl_array_insert(&bml_endpoint->btl_send);
                bml_btl->btl = btl;
                bml_btl->btl_eager_limit = btl->btl_eager_limit;
                bml_btl->btl_min_send_size = btl->btl_min_send_size;
                bml_btl->btl_max_send_size = btl->btl_max_send_size;
                bml_btl->btl_min_rdma_size = btl->btl_min_rdma_size;
                bml_btl->btl_max_rdma_size = btl->btl_max_rdma_size;
                bml_btl->btl_cache = NULL;
                bml_btl->btl_endpoint = btl_endpoints[p];
                bml_btl->btl_weight = 0;
                bml_btl->btl_alloc = btl->btl_alloc;
                bml_btl->btl_free = btl->btl_free;
                bml_btl->btl_prepare_src = btl->btl_prepare_src;
                bml_btl->btl_prepare_dst = btl->btl_prepare_dst;
                bml_btl->btl_send = btl->btl_send;
                bml_btl->btl_put = btl->btl_put;
                bml_btl->btl_get = btl->btl_get;
                bml_btl->btl_progress = btl->btl_component->btl_progress; 
                
                
                bml_endpoints[p]=bml_endpoint; 
                proc->proc_pml = (mca_pml_proc_t*) bml_endpoint; 
                /* orte_hash_table_set_proc(  */
/*                                          &mca_bml_r2.procs,  */
/*                                          &proc->proc_name,  */
/*                                          proc);  */
                
            }
        }
        if(btl_inuse > 0 && NULL != btl->btl_component->btl_progress) {
            size_t p;
            bool found = false;
            for(p=0; p<mca_bml_r2.num_btl_progress; p++) {
                if(mca_bml_r2.btl_progress[p] == btl->btl_component->btl_progress) {
                    found = true;
                    break;
                }
            }
            if(found == false) {
                mca_bml_r2.btl_progress[mca_bml_r2.num_btl_progress] = 
                    btl->btl_component->btl_progress;
                mca_bml_r2.num_btl_progress++;
            }
        }
    }
    free(btl_endpoints);

    /* iterate back through procs and compute metrics for registered r2s */
    for(p=0; p<nprocs; p++) {
        ompi_proc_t *proc = procs[p];
        mca_bml_base_endpoint_t* bml_endpoint = (mca_bml_base_endpoint_t*) proc->proc_pml;
        double total_bandwidth = 0;
        uint32_t latency = 0;
        size_t n_index;
        size_t n_size;

        /* skip over procs w/ no ptls registered */
        if(NULL == bml_endpoint)
            continue;

        /* (1) determine the total bandwidth available across all r2s
         *     note that we need to do this here, as we may already have r2s configured
         * (2) determine the highest priority ranking for latency
         */
        n_size = mca_bml_base_btl_array_get_size(&bml_endpoint->btl_send); 
        for(n_index = 0; n_index < n_size; n_index++) {
            mca_bml_base_btl_t* bml_btl = 
                mca_bml_base_btl_array_get_index(&bml_endpoint->btl_send, n_index);
            mca_btl_base_module_t* btl = bml_btl->btl;
            total_bandwidth += bml_btl->btl->btl_bandwidth; 
            if(btl->btl_latency > latency)
                latency = btl->btl_latency;
        }

        /* (1) set the weight of each btl as a percentage of overall bandwidth
         * (2) copy all btl instances at the highest priority ranking into the
         *     list of btls used for first fragments
         */

        for(n_index = 0; n_index < n_size; n_index++) {
            mca_bml_base_btl_t* bml_btl = 
                mca_bml_base_btl_array_get_index(&bml_endpoint->btl_send, n_index);
            mca_btl_base_module_t *btl = bml_btl->btl;
            double weight;

            /* compute weighting factor for this r2 */
            if(btl->btl_bandwidth)
                weight = btl->btl_bandwidth / total_bandwidth;
            else
                weight = 1.0 / n_size;
            bml_btl->btl_weight = (int)(weight * 100);

            /* check to see if this r2 is already in the array of r2s 
             * used for first fragments - if not add it.
             */
            if(btl->btl_latency == latency) {
                mca_bml_base_btl_t* bml_btl_new = 
                    mca_bml_base_btl_array_insert(&bml_endpoint->btl_eager);
                *bml_btl_new = *bml_btl;
            }

            /* check flags - is rdma prefered */
            if(btl->btl_flags & MCA_BTL_FLAGS_RDMA &&
               proc->proc_arch == ompi_proc_local_proc->proc_arch) {
                mca_bml_base_btl_t* bml_btl_rdma = mca_bml_base_btl_array_insert(&bml_endpoint->btl_rdma);
                *bml_btl_rdma = *bml_btl;
                if(bml_endpoint->btl_rdma_offset < bml_btl_rdma->btl_min_rdma_size) {
                    bml_endpoint->btl_rdma_offset = bml_btl_rdma->btl_min_rdma_size;
                }
            }
        }
    }
    return OMPI_SUCCESS;
}

/*
 * iterate through each proc and notify any PTLs associated
 * with the proc that it is/has gone away
 */

int mca_bml_r2_del_procs(size_t nprocs, 
                         struct ompi_proc_t** procs) 
{
    size_t p;
    int rc;
    for(p = 0; p < nprocs; p++) {
        ompi_proc_t *proc = procs[p];
        mca_bml_base_endpoint_t* bml_endpoint = (mca_bml_base_endpoint_t*) proc->proc_pml;
        size_t f_index, f_size;
        size_t n_index, n_size;
 
        /* notify each ptl that the proc is going away */
        f_size = mca_bml_base_btl_array_get_size(&bml_endpoint->btl_eager);
        for(f_index = 0; f_index < f_size; f_index++) {
            mca_bml_base_btl_t* bml_btl = mca_bml_base_btl_array_get_index(&bml_endpoint->btl_eager, f_index);
            mca_btl_base_module_t* btl = bml_btl->btl;
            
            rc = btl->btl_del_procs(btl,1,&proc,&bml_btl->btl_endpoint);
            if(OMPI_SUCCESS != rc) {
                return rc;
            }

            /* remove this from next array so that we dont call it twice w/ 
             * the same address pointer
             */
            n_size = mca_bml_base_btl_array_get_size(&bml_endpoint->btl_eager);
            for(n_index = 0; n_index < n_size; n_index++) {
                mca_bml_base_btl_t* bml_btl = mca_bml_base_btl_array_get_index(&bml_endpoint->btl_send, n_index);
                if(bml_btl->btl == btl) {
                    memset(bml_btl, 0, sizeof(mca_bml_base_btl_t));
                    break;
                }
            }
        }

        /* notify each r2 that was not in the array of r2s for first fragments */
        n_size = mca_bml_base_btl_array_get_size(&bml_endpoint->btl_send);
        for(n_index = 0; n_index < n_size; n_index++) {
            mca_bml_base_btl_t* bml_btl = mca_bml_base_btl_array_get_index(&bml_endpoint->btl_eager, n_index);
            mca_btl_base_module_t* btl = bml_btl->btl;
            if (btl != 0) {
                rc = btl->btl_del_procs(btl,1,&proc,&bml_btl->btl_endpoint);
                if(OMPI_SUCCESS != rc)
                    return rc;
            }
        }
        
        /* do any required cleanup */
        OBJ_RELEASE(bml_endpoint);
        
    }
    return OMPI_SUCCESS;
}

int mca_bml_r2_finalize( void ) { 
    return OMPI_SUCCESS; /* TODO */ 
} 

int mca_bml_r2_register( 
                        mca_btl_base_tag_t tag, 
                        mca_bml_base_module_recv_cb_fn_t cbfunc, 
                        void* data
                        )
{
    uint32_t  i; 
    int rc;
    mca_btl_base_module_t *btl; 

    for(i = 0; i < mca_bml_r2.num_btl_modules; i++) { 
        btl = mca_bml_r2.btl_modules[i]; 
        rc = btl->btl_register(btl, tag, cbfunc, data);  
        if(OMPI_SUCCESS != rc)
            return rc;
    }
    return OMPI_SUCCESS; 
}


int mca_bml_r2_component_fini(void)
{
    /* FIX */
    return OMPI_SUCCESS;
}

