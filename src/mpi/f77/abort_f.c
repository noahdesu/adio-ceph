/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_ABORT = mpi_abort_f
#pragma weak pmpi_abort = mpi_abort_f
#pragma weak pmpi_abort_ = mpi_abort_f
#pragma weak pmpi_abort__ = mpi_abort_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_ABORT,
                           pmpi_abort,
                           pmpi_abort_,
                           pmpi_abort__,
                           pmpi_abort_f,
                           (MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr),
                           (comm, errorcode, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_ABORT = mpi_abort_f
#pragma weak mpi_abort = mpi_abort_f
#pragma weak mpi_abort_ = mpi_abort_f
#pragma weak mpi_abort__ = mpi_abort_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_ABORT,
                           mpi_abort,
                           mpi_abort_,
                           mpi_abort__,
                           mpi_abort_f,
                           (MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr),
                           (comm, errorcode, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/f77/profile/defines.h"
#endif

void mpi_abort_f(MPI_Fint *comm, MPI_Fint *errorcode, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    
    *ierr = OMPI_INT_2_FINT(MPI_Abort(c_comm, OMPI_FINT_2_INT(*errorcode)));
}
