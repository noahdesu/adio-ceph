/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"
#include "mpi/f77/constants.h"
#include "errhandler/errhandler.h"
#include "communicator/communicator.h"
#include "mpi/runtime/params.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_RECV = mpi_recv_f
#pragma weak pmpi_recv = mpi_recv_f
#pragma weak pmpi_recv_ = mpi_recv_f
#pragma weak pmpi_recv__ = mpi_recv_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_RECV,
                           pmpi_recv,
                           pmpi_recv_,
                           pmpi_recv__,
                           pmpi_recv_f,
                           (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr),
                           (buf, count, datatype, source, tag, comm, status, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_RECV = mpi_recv_f
#pragma weak mpi_recv = mpi_recv_f
#pragma weak mpi_recv_ = mpi_recv_f
#pragma weak mpi_recv__ = mpi_recv_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_RECV,
                           mpi_recv,
                           mpi_recv_,
                           mpi_recv__,
                           mpi_recv_f,
                           (char *buf, MPI_Fint *count, MPI_Fint *datatype, MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, MPI_Fint *status, MPI_Fint *ierr),
                           (buf, count, datatype, source, tag, comm, status, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/f77/profile/defines.h"
#endif

void mpi_recv_f(char *buf, MPI_Fint *count, MPI_Fint *datatype, 
                MPI_Fint *source, MPI_Fint *tag, MPI_Fint *comm, 
                MPI_Fint *status, MPI_Fint *ierr)
{
    MPI_Status *c_status;
    int c_err;
#if OMPI_SIZEOF_FORTRAN_INT != SIZEOF_INT
    MPI_Status c_status2;
#endif
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    MPI_Datatype c_type = MPI_Type_f2c(*datatype);

    /* Only check for the bad value if we're checking MPI parameters */

    if (MPI_PARAM_CHECK) {
        if (OMPI_IS_FORTRAN_STATUSES_IGNORE(status)) {
            c_err = OMPI_ERRHANDLER_INVOKE(c_comm, MPI_ERR_ARG,
					   "MPI_RECV");
	    *ierr = OMPI_INT_2_FINT(c_err);
            return;
        }
    }

    /* See if we got MPI_STATUS_IGNORE */

    if (OMPI_IS_FORTRAN_STATUS_IGNORE(status)) {
        c_status = MPI_STATUS_IGNORE;
    } else {

        /* If sizeof(int) == sizeof(INTEGER), then there's no
           translation necessary -- let the underlying functions write
           directly into the Fortran status */

#if OMPI_SIZEOF_FORTRAN_INT == SIZEOF_INT
        c_status = (MPI_Status *) status;
#else
        c_status = &c_status2;
#endif
    }

    /* Call the C function */

    *ierr = OMPI_INT_2_FINT(MPI_Recv(buf, OMPI_FINT_2_INT(*count), c_type, 
                                     OMPI_FINT_2_INT(*source), 
                                     OMPI_FINT_2_INT(*tag), c_comm,
                                     c_status));
#if OMPI_SIZEOF_FORTRAN_INT != SIZEOF_INT
    if (MPI_STATUS_IGNORE != c_status) {
        MPI_Status_c2f(c_status, status);
    }
#endif
}
