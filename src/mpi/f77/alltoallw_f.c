/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_ALLTOALLW = mpi_alltoallw_f
#pragma weak pmpi_alltoallw = mpi_alltoallw_f
#pragma weak pmpi_alltoallw_ = mpi_alltoallw_f
#pragma weak pmpi_alltoallw__ = mpi_alltoallw_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_ALLTOALLW,
                           pmpi_alltoallw,
                           pmpi_alltoallw_,
                           pmpi_alltoallw__,
                           pmpi_alltoallw_f,
                           (char *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtypes, char *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtypes, MPI_Fint *comm, MPI_Fint *ierr),
                           (sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_ALLTOALLW = mpi_alltoallw_f
#pragma weak mpi_alltoallw = mpi_alltoallw_f
#pragma weak mpi_alltoallw_ = mpi_alltoallw_f
#pragma weak mpi_alltoallw__ = mpi_alltoallw_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_ALLTOALLW,
                           mpi_alltoallw,
                           mpi_alltoallw_,
                           mpi_alltoallw__,
                           mpi_alltoallw_f,
                           (char *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtypes, char *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtypes, MPI_Fint *comm, MPI_Fint *ierr),
                           (sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/f77/profile/defines.h"
#endif

void mpi_alltoallw_f(char *sendbuf, MPI_Fint *sendcounts,
		     MPI_Fint *sdispls, MPI_Fint *sendtypes, 
		     char *recvbuf, MPI_Fint *recvcounts,
		     MPI_Fint *rdispls, MPI_Fint *recvtypes,
		     MPI_Fint *comm, MPI_Fint *ierr)
{
    MPI_Comm c_comm;
    MPI_Datatype *c_sendtypes, *c_recvtypes;
    int size;
    OMPI_ARRAY_NAME_DECL(sendcounts);
    OMPI_ARRAY_NAME_DECL(sdispls);
    OMPI_ARRAY_NAME_DECL(recvcounts);
    OMPI_ARRAY_NAME_DECL(rdispls);

    c_comm = MPI_Comm_f2c(*comm);
    MPI_Comm_size(c_comm, &size);

    c_sendtypes = malloc(size * sizeof(MPI_Datatype));
    c_recvtypes = malloc(size * sizeof(MPI_Datatype));

    OMPI_ARRAY_FINT_2_INT(sendcounts, size);
    OMPI_ARRAY_FINT_2_INT(sdispls, size);
    OMPI_ARRAY_FINT_2_INT(recvcounts, size);
    OMPI_ARRAY_FINT_2_INT(rdispls, size);

    while (size > 0) {
	c_sendtypes[size - 1] = MPI_Type_f2c(sendtypes[size - 1]);
	c_recvtypes[size - 1] = MPI_Type_f2c(recvtypes[size - 1]);
	--size;
    }

    *ierr = OMPI_INT_2_FINT(MPI_Alltoallw(sendbuf, 
					  OMPI_ARRAY_NAME_CONVERT(sendcounts),
					  OMPI_ARRAY_NAME_CONVERT(sdispls),
					  c_sendtypes, 
					  recvbuf, 
					  OMPI_ARRAY_NAME_CONVERT(recvcounts),
					  OMPI_ARRAY_NAME_CONVERT(rdispls),
					  c_recvtypes, c_comm));

    OMPI_ARRAY_FINT_2_INT_CLEANUP(sendcounts);
    OMPI_ARRAY_FINT_2_INT_CLEANUP(sdispls);
    OMPI_ARRAY_FINT_2_INT_CLEANUP(recvcounts);
    OMPI_ARRAY_FINT_2_INT_CLEANUP(rdispls);
}
