/*
 * $HEADER$
 */

#include "lam_config.h"
#include <stdio.h>

#include "mpi.h"
#include "mpi/interface/c/bindings.h"

#if LAM_HAVE_WEAK_SYMBOLS && LAM_PROFILING_DEFINES
#pragma weak MPI_Allgatherv = PMPI_Allgatherv
#endif

int MPI_Allgatherv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
		           void *recvbuf, int *recvcounts,
				   int *displs, MPI_Datatype recvtype, MPI_Comm comm) {

	return MPI_SUCCESS;
}

