/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "ompi_config.h"
#include "datatype/datatype.h"

/* Open questions ...
 *  - how to improuve the handling of these vectors (creating a temporary datatype
 *    can be ONLY a initial solution.
 *
 */

int32_t ompi_ddt_create_vector( int count, int bLength, long stride,
                                const dt_desc_t* oldType, dt_desc_t** newType )
{
    long extent = oldType->ub - oldType->lb;
    dt_desc_t *pTempData, *pData;

    pTempData = ompi_ddt_create( oldType->desc.used + 2 );
    if( (bLength == stride) || (1 >= count) ) {  /* the elements are contiguous */
        pData = pTempData;
        ompi_ddt_add( pData, oldType, count * bLength, 0, extent );
    } else {
        if( 1 == bLength ) {
            pData = pTempData;
            ompi_ddt_add( pData, oldType, count, 0, extent * stride );
        } else {
            ompi_ddt_add( pTempData, oldType, bLength, 0, extent );
            pData = ompi_ddt_create( oldType->desc.used + 2 + 2 );
            ompi_ddt_add( pData, pTempData, count, 0, extent * stride );
        }
        if( 1 != bLength )
            OBJ_RELEASE( pTempData );
        /* correct the ub to remove the last stride */
        pData->ub -= (stride - bLength) * extent;
    }
    *newType = pData;
    return OMPI_SUCCESS;
}

int32_t ompi_ddt_create_hvector( int count, int bLength, long stride,
                                 const dt_desc_t* oldType, dt_desc_t** newType )
{
    long extent = oldType->ub - oldType->lb;
    dt_desc_t *pTempData, *pData;

    pTempData = ompi_ddt_create( oldType->desc.used + 2 );
    if( ((extent * bLength) == stride) || (1 >= count) ) {  /* contiguous */
        pData = pTempData;
        ompi_ddt_add( pData, oldType, count * bLength, 0, extent );
    } else {
        if( 1 == bLength ) {
            pData = pTempData;
            ompi_ddt_add( pData, oldType, count, 0, stride );
        } else {
            ompi_ddt_add( pTempData, oldType, bLength, 0, extent );
            pData = ompi_ddt_create( oldType->desc.used + 2 + 2 );
            ompi_ddt_add( pData, pTempData, count, 0, stride );
        }
        if( 1 != bLength )
            OBJ_RELEASE( pTempData );
        /* correct the ub to remove the last stride */
        pData->ub -= (stride - (bLength * extent));
    }
    *newType = pData;
    return OMPI_SUCCESS;
}
