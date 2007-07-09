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
 * Copyright (c) 2006-2007 Los Alamos National Security, LLC.  All rights
 *                         reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

/** @file
 * Open MPI module-related data transfer mechanism
 *
 * A system for publishing module-related data for global
 * initialization.  Known simply as the "modex", this interface
 * provides a system for sharing data, particularly data related to
 * modules and their availability on the system.
 *
 * The modex system is tightly integrated into the general run-time
 * initialization system and takes advantage of global update periods
 * to minimize the amount of network traffic.  All updates are also
 * stored in the general purpose registry, and can be read at any time
 * during the life of the process.  Care should be taken to not call
 * the blocking receive during the first stage of global
 * initialization, as data will not be available the process will
 * likely hang.
 *
 * @note For the purpose of this interface, two components are
 * "corresponding" if:
 *   - they share the same major and minor MCA version number
 *   - they have the same type name string
 *   - they share the same major and minor type version number
 *   - they have the same component name string
 *   - they share the same major and minor component version number
 */

#ifndef MCA_OMPI_MODULE_EXCHANGE_H
#define MCA_OMPI_MODULE_EXCHANGE_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "orte/mca/ns/ns_types.h"

struct mca_base_component_t;
struct ompi_proc_t;

BEGIN_C_DECLS

/**
 * Send a module-specific buffer to all other corresponding MCA
 * modules in peer processes
 * 
 * This function takes a contiguous buffer of network-ordered data
 * and makes it available to all other MCA processes during the
 * selection process.  Modules sent by one source_component can only
 * be received by a corresponding module with the same
 * component name.
 *
 * This function is indended to be used during MCA module
 * initialization \em before \em selection (the selection process is
 * defined differently for each component type).  Each module will
 * provide a buffer containing meta information and/or parameters
 * that it wants to share with its corresponding modules in peer
 * processes.  This information typically contains location /
 * contact information for establishing communication between
 * processes (in a manner that is specific to that module).  For
 * example, a TCP-based module could provide its IP address and TCP
 * port where it is waiting on listen().  The peer process receiving
 * this buffer can therefore open a socket to the indicated IP
 * address and TCP port.
 *
 * During the selection process, the MCA framework will effectively
 * perform an "allgather" operation of all modex buffers; every
 * buffer will be available to every peer process (see
 * ompi_modex_recv()).
 *
 * The buffer is copied during the send call and may be modified or
 * free()'ed immediately after the return from this function call.
 *
 * @note Buffer contents is transparent to the MCA framework -- it \em
 * must already either be in network order or be in some format that
 * peer processes will be able to read it, regardless of pointer sizes
 * or endian bias.
 *
 * @param source_component A pointer to this module's component
 *                         structure
 * @param buffer           A pointer to the beginning of the buffer to send
 * @param size             Number of bytes in the buffer
 *
 * @retval OMPI_SUCCESS On success
 * @retval OMPI_ERROR   An unspecified error occurred
 */
OMPI_DECLSPEC int ompi_modex_send(struct mca_base_component_t *source_component, 
                                  const void *buffer, size_t size);


/**
 * Receive a module-specific buffer from a corresponding MCA module
 * in a specific peer process
 *
 * This is the corresponding "get" call to ompi_modex_send().
 * After selection, modules can call this function to receive the
 * buffer sent by their corresponding module on the process
 * source_proc.
 *
 * If a buffer from a corresponding module is found, buffer will be
 * filled with a pointer to a copy of the buffer that was sent by
 * the peer process.  It is the caller's responsibility to free this
 * buffer.  The size will be filled in with the total size of the
 * buffer.
 *
 * @note If the modex system has received information from a given
 * process, but has not yet received information for the given
 * component, ompi_modex_recv() will return no data.  This
 * can not happen to a process that has gone through the normal
 * startup proceedure, but if you believe this can happen with your
 * component, you should use ompi_modex_recv_nb() to receive updates
 * when the information becomes available.
 *
 * @param dest_component A pointer to this module's component struct
 * @param source_proc    Peer process to receive from
 * @param buffer         A pointer to a (void*) that will be filled
 *                       with a pointer to the received buffer
 * @param size           Pointer to a size_t that will be filled with
 *                       the number of bytes in the buffer
 *
 * @retval OMPI_SUCCESS If a corresponding module buffer is found and
 *                      successfully returned to the caller.
 * @retval OMPI_ERR_NOT_IMPLEMENTED Modex support is not available in
 *                      this build of Open MPI (systems like the Cray XT)
 * @retval OMPI_ERR_OUT_OF_RESOURCE No memory could be allocated for the
 *                       buffer.
 */
OMPI_DECLSPEC int ompi_modex_recv(struct mca_base_component_t *dest_component,
                                  struct ompi_proc_t *source_proc,
                                  void **buffer, size_t *size);

typedef void (*ompi_modex_cb_fn_t)(struct mca_base_component_t *component,
                                   struct ompi_proc_t* proc,
                                   void* buffer,
                                   size_t size,
                                   void* cbdata);

/**
 * Register to receive a callback on change to module specific data.
 *
 * The non-blocking version of ompi_modex_recv().  All information
 * about ompi_modex_recv() applies to ompi_modex_recv_nb(), with the
 * exception of what happens when data is available for the given peer
 * process but not the specified module.  In that case, no callback
 * will be fired until data is available.
 *
 * @param component      A pointer to this module's component struct
 * @param proc           Peer process to receive from
 * @param cbfunc         Callback function when data is available,
 *                       of type ompi_modex_cb_fn_t
 * @param cbdata         Opaque callback data to pass to cbfunc
 *
 * @retval OMPI_SUCCESS  Success
 * @retval OMPI_ERR_OUT_OF_RESOURCE No memory could be allocated
 *                       for internal data structures
 */
OMPI_DECLSPEC int ompi_modex_recv_nb(struct mca_base_component_t *component,
                                     struct ompi_proc_t* proc,
                                     ompi_modex_cb_fn_t cbfunc,
                                     void* cbdata);


 /**
  * Subscribe to resource updates for a specific job
  *
  * Generally called during process initialization, after all the data
  * has been loaded into the module exchange system, but before the
  * data is actually used.
  * 
  * Intended to help the scalability of start-up by not subscribing to
  * the job updates until all data is in the system (and not firing
  * updates along the way) and launching the asynchronous request for
  * the data before it is actually needed later in init.
  *
  * This function is probably not useful outside of application
  * initialization code.
  */
OMPI_DECLSPEC int ompi_modex_subscribe_job(orte_jobid_t jobid);


/**
 * Initialize the modex system
 *
 * Allocate memory for the local data cache and initialize the
 * module exchange system.  Does not cause communication nor any
 * subscriptions to be placed on the registry.
 */
OMPI_DECLSPEC int ompi_modex_init(void);


/**
 * Finalize the modex system
 *
 * Release any memory associated with the modex system, remove all
 * subscriptions on the GPR and end all non-blocking update triggers
 * currently available on the system.
 */
OMPI_DECLSPEC int ompi_modex_finalize(void);

END_C_DECLS

#endif /* MCA_OMPI_MODULE_EXCHANGE_H */
