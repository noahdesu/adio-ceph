/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2008 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007      Cisco Systems, Inc.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "ompi_config.h"
#include <stdio.h>

#include "ompi/mpi/c/bindings.h"
#include "ompi/datatype/datatype.h"
#include "ompi/memchecker.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILING_DEFINES
#pragma weak MPI_Allgather = PMPI_Allgather
#endif

#if OMPI_PROFILING_DEFINES
#include "ompi/mpi/c/profile/defines.h"
#endif

static const char FUNC_NAME[] = "MPI_Allgather";


int MPI_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
                  void *recvbuf, int recvcount, MPI_Datatype recvtype,
                  MPI_Comm comm)
{
    int err;

    MEMCHECKER(
        int rank;
        ptrdiff_t ext;

        rank = ompi_comm_rank(comm);
        ompi_ddt_type_extent(recvtype, &ext);

        memchecker_datatype(recvtype);
        memchecker_comm(comm);
        /* check whether the actual send buffer is defined. */
        if (MPI_IN_PLACE == sendbuf) {
            memchecker_call(&opal_memchecker_base_isdefined, 
                            (char *)(recvbuf)+rank*ext, 
                            recvcount, recvtype);
        } else {
            memchecker_datatype(sendtype);
            memchecker_call(&opal_memchecker_base_isdefined, sendbuf, sendcount, sendtype);
        }
        /* check whether the receive buffer is addressable. */
        memchecker_call(&opal_memchecker_base_isaddressable, recvbuf, recvcount, recvtype);
    );

    if (MPI_PARAM_CHECK) {

        /* Unrooted operation -- same checks for all ranks on both
           intracommunicators and intercommunicators */

        err = MPI_SUCCESS;
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
          OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        } else if (MPI_DATATYPE_NULL == recvtype || NULL == recvtype) {
          err = MPI_ERR_TYPE;
        } else if (recvcount < 0) {
          err = MPI_ERR_COUNT;
        } else if (MPI_IN_PLACE == recvbuf) {
          return OMPI_ERRHANDLER_INVOKE(comm, MPI_ERR_ARG, FUNC_NAME);
        } else if (MPI_IN_PLACE != sendbuf) {
            OMPI_CHECK_DATATYPE_FOR_SEND(err, sendtype, sendcount);
        }
        OMPI_ERRHANDLER_CHECK(err, comm, err, FUNC_NAME);
    }

    /* Do we need to do anything?  Everyone had to give the same send
       signature, which means that everyone must have given a
       sendcount > 0 if there's anything to send.  If we're doing
       IN_PLACE, however, check recvcount, not sendcount. */

    if ((MPI_IN_PLACE != sendbuf && 0 == sendcount) ||
        (0 == recvcount)) {
        return MPI_SUCCESS;
    }

    OPAL_CR_ENTER_LIBRARY();

    /* Invoke the coll component to perform the back-end operation */

    err = comm->c_coll.coll_allgather(sendbuf, sendcount, sendtype, 
                                      recvbuf, recvcount, recvtype, comm,
                                      comm->c_coll.coll_allgather_module);
    OMPI_ERRHANDLER_RETURN(err, comm, err, FUNC_NAME);
}

