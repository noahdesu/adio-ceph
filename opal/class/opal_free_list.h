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

#ifndef OMPI_FREE_LIST_H
#define OMPI_FREE_LIST_H

#include "ompi_config.h"
#include "class/ompi_list.h"
#include "threads/thread.h"
#include "threads/condition.h"
#include "include/constants.h"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif
OMPI_DECLSPEC extern ompi_class_t opal_free_list_t_class;


struct opal_free_list_t
{
    ompi_list_t super;
    size_t fl_max_to_alloc;
    size_t fl_num_allocated;
    size_t fl_num_per_alloc;
    size_t fl_num_waiting;
    size_t fl_elem_size;
    ompi_class_t* fl_elem_class;
    ompi_mutex_t fl_lock;
    ompi_condition_t fl_condition; 
};
typedef struct opal_free_list_t opal_free_list_t;

struct opal_free_list_item_t
{ 
    ompi_list_item_t super; 
    void* user_data; 
}; 
typedef struct opal_free_list_item_t opal_free_list_item_t; 

/**
 * Initialize a free list.
 *
 * @param free_list (IN)           Free list.
 * @param element_size (IN)        Size of each element.
 * @param element_class (IN)       ompi_class_t of element - used to initialize allocated elements.
 * @param num_elements_to_alloc    Initial number of elements to allocate.
 * @param max_elements_to_alloc    Maximum number of elements to allocate.
 * @param num_elements_per_alloc   Number of elements to grow by per allocation.
 */
 
OMPI_DECLSPEC int opal_free_list_init(
    opal_free_list_t *free_list, 
    size_t element_size,
    ompi_class_t* element_class,
    int num_elements_to_alloc,
    int max_elements_to_alloc,
    int num_elements_per_alloc);

OMPI_DECLSPEC int opal_free_list_grow(opal_free_list_t* flist, size_t num_elements);
    
/**
 * Attemp to obtain an item from a free list. 
 *
 * @param fl (IN)        Free list.
 * @param item (OUT)     Allocated item.
 * @param rc (OUT)       OMPI_SUCCESS or error status on failure.
 *
 * If the requested item is not available the free list is grown to 
 * accomodate the request - unless the max number of allocations has 
 * been reached.  If this is the case - an out of resource error is 
 * returned to the caller.
 */
 
#define OPAL_FREE_LIST_GET(fl, item, rc) \
{ \
    if(ompi_using_threads()) { \
        ompi_mutex_lock(&((fl)->fl_lock)); \
        item = ompi_list_remove_first(&((fl)->super)); \
        if(NULL == item) { \
            opal_free_list_grow((fl), (fl)->fl_num_per_alloc); \
            item = ompi_list_remove_first(&((fl)->super)); \
        } \
        ompi_mutex_unlock(&((fl)->fl_lock)); \
    } else { \
        item = ompi_list_remove_first(&((fl)->super)); \
        if(NULL == item) { \
            opal_free_list_grow((fl), (fl)->fl_num_per_alloc); \
            item = ompi_list_remove_first(&((fl)->super)); \
        } \
    }  \
    rc = (NULL == item) ?  OMPI_ERR_TEMP_OUT_OF_RESOURCE : OMPI_SUCCESS; \
} 

/**
 * Blocking call to obtain an item from a free list.
 *
 * @param fl (IN)        Free list.
 * @param item (OUT)     Allocated item.
 * @param rc (OUT)       OMPI_SUCCESS or error status on failure.
 *
 * If the requested item is not available the free list is grown to 
 * accomodate the request - unless the max number of allocations has 
 * been reached. In this case the caller is blocked until an item
 * is returned to the list.
 */
 

#define OPAL_FREE_LIST_WAIT(fl, item, rc)                                  \
{                                                                          \
    OMPI_THREAD_LOCK(&((fl)->fl_lock));                                    \
    item = ompi_list_remove_first(&((fl)->super));                         \
    while(NULL == item) {                                                  \
        if((fl)->fl_max_to_alloc <= (fl)->fl_num_allocated) {              \
            (fl)->fl_num_waiting++;                                        \
            ompi_condition_wait(&((fl)->fl_condition), &((fl)->fl_lock));  \
            (fl)->fl_num_waiting--;                                        \
        } else {                                                           \
            opal_free_list_grow((fl), (fl)->fl_num_per_alloc);             \
        }                                                                  \
        item = ompi_list_remove_first(&((fl)->super));                     \
    }                                                                      \
    OMPI_THREAD_UNLOCK(&((fl)->fl_lock));                                  \
    rc = (NULL == item) ?  OMPI_ERR_OUT_OF_RESOURCE : OMPI_SUCCESS;        \
} 


/**
 * Return an item to a free list. 
 *
 * @param fl (IN)        Free list.
 * @param item (OUT)     Allocated item.
 *
 */
 
#define OPAL_FREE_LIST_RETURN(fl, item)                                    \
{                                                                          \
    OMPI_THREAD_LOCK(&(fl)->fl_lock);                                      \
    ompi_list_prepend(&((fl)->super), (item));                            \
    if((fl)->fl_num_waiting > 0) {                                         \
        ompi_condition_signal(&((fl)->fl_condition));                      \
    }                                                                      \
    OMPI_THREAD_UNLOCK(&(fl)->fl_lock);                                    \
}
#if defined(c_plusplus) || defined(__cplusplus)
}
#endif
#endif 

