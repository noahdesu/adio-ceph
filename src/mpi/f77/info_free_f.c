/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_INFO_FREE = mpi_info_free_f
#pragma weak pmpi_info_free = mpi_info_free_f
#pragma weak pmpi_info_free_ = mpi_info_free_f
#pragma weak pmpi_info_free__ = mpi_info_free_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_INFO_FREE,
                           pmpi_info_free,
                           pmpi_info_free_,
                           pmpi_info_free__,
                           pmpi_info_free_f,
                           (MPI_Fint *info, MPI_Fint *ierr),
                           (info, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_INFO_FREE = mpi_info_free_f
#pragma weak mpi_info_free = mpi_info_free_f
#pragma weak mpi_info_free_ = mpi_info_free_f
#pragma weak mpi_info_free__ = mpi_info_free_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_INFO_FREE,
                           mpi_info_free,
                           mpi_info_free_,
                           mpi_info_free__,
                           mpi_info_free_f,
                           (MPI_Fint *info, MPI_Fint *ierr),
                           (info, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/c/profile/defines.h"
#endif

void mpi_info_free_f(MPI_Fint *info, MPI_Fint *ierr)
{

}
