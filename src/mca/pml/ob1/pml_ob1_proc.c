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

#include "include/sys/atomic.h"
#include "pml_ob1.h"
#include "pml_ob1_proc.h"


static void mca_pml_ob1_proc_construct(mca_pml_ob1_proc_t* proc)
{
    proc->proc_ompi = NULL;
    proc->proc_sequence = 0;
    OBJ_CONSTRUCT(&proc->proc_lock, ompi_mutex_t);
    OBJ_CONSTRUCT(&proc->bmi_eager, mca_pml_ob1_ep_array_t);
    OBJ_CONSTRUCT(&proc->bmi_send,  mca_pml_ob1_ep_array_t);
    OBJ_CONSTRUCT(&proc->bmi_rdma,  mca_pml_ob1_ep_array_t);
}


static void mca_pml_ob1_proc_destruct(mca_pml_ob1_proc_t* proc)
{
    OBJ_DESTRUCT(&proc->proc_lock);
    OBJ_DESTRUCT(&proc->bmi_eager);
    OBJ_DESTRUCT(&proc->bmi_send);
    OBJ_DESTRUCT(&proc->bmi_rdma);
}


OBJ_CLASS_INSTANCE(
    mca_pml_ob1_proc_t,
    ompi_object_t,
    mca_pml_ob1_proc_construct, 
    mca_pml_ob1_proc_destruct 
);

