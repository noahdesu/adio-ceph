/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_CART_SUB = mpi_cart_sub_f
#pragma weak pmpi_cart_sub = mpi_cart_sub_f
#pragma weak pmpi_cart_sub_ = mpi_cart_sub_f
#pragma weak pmpi_cart_sub__ = mpi_cart_sub_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_CART_SUB,
                           pmpi_cart_sub,
                           pmpi_cart_sub_,
                           pmpi_cart_sub__,
                           pmpi_cart_sub_f,
                           (MPI_Fint *comm, MPI_Fint *remain_dims, MPI_Fint *new_comm, MPI_Fint *ierr),
                           (comm, remain_dims, new_comm, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_CART_SUB = mpi_cart_sub_f
#pragma weak mpi_cart_sub = mpi_cart_sub_f
#pragma weak mpi_cart_sub_ = mpi_cart_sub_f
#pragma weak mpi_cart_sub__ = mpi_cart_sub_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_CART_SUB,
                           mpi_cart_sub,
                           mpi_cart_sub_,
                           mpi_cart_sub__,
                           mpi_cart_sub_f,
                           (MPI_Fint *comm, MPI_Fint *remain_dims, MPI_Fint *new_comm, MPI_Fint *ierr),
                           (comm, remain_dims, new_comm, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/c/profile/defines.h"
#endif

void mpi_cart_sub_f(MPI_Fint *comm, MPI_Fint *remain_dims, MPI_Fint *new_comm, MPI_Fint *ierr)
{

}
