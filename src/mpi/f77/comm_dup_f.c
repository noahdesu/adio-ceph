/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_COMM_DUP = mpi_comm_dup_f
#pragma weak pmpi_comm_dup = mpi_comm_dup_f
#pragma weak pmpi_comm_dup_ = mpi_comm_dup_f
#pragma weak pmpi_comm_dup__ = mpi_comm_dup_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_COMM_DUP,
                           pmpi_comm_dup,
                           pmpi_comm_dup_,
                           pmpi_comm_dup__,
                           pmpi_comm_dup_f,
                           (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr),
                           (comm, newcomm, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_COMM_DUP = mpi_comm_dup_f
#pragma weak mpi_comm_dup = mpi_comm_dup_f
#pragma weak mpi_comm_dup_ = mpi_comm_dup_f
#pragma weak mpi_comm_dup__ = mpi_comm_dup_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_COMM_DUP,
                           mpi_comm_dup,
                           mpi_comm_dup_,
                           mpi_comm_dup__,
                           mpi_comm_dup_f,
                           (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr),
                           (comm, newcomm, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/f77/profile/defines.h"
#endif

OMPI_EXPORT
void mpi_comm_dup_f(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr)
{
    MPI_Comm c_newcomm;
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    
    *ierr = OMPI_INT_2_FINT(MPI_Comm_dup(c_comm, &c_newcomm));
    *newcomm = MPI_Comm_c2f(c_newcomm);
}
