/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_CART_SHIFT = mpi_cart_shift_f
#pragma weak pmpi_cart_shift = mpi_cart_shift_f
#pragma weak pmpi_cart_shift_ = mpi_cart_shift_f
#pragma weak pmpi_cart_shift__ = mpi_cart_shift_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_CART_SHIFT,
                           pmpi_cart_shift,
                           pmpi_cart_shift_,
                           pmpi_cart_shift__,
                           pmpi_cart_shift_f,
                           (MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr),
                           (comm, direction, disp, rank_source, rank_dest, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_CART_SHIFT = mpi_cart_shift_f
#pragma weak mpi_cart_shift = mpi_cart_shift_f
#pragma weak mpi_cart_shift_ = mpi_cart_shift_f
#pragma weak mpi_cart_shift__ = mpi_cart_shift_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_CART_SHIFT,
                           mpi_cart_shift,
                           mpi_cart_shift_,
                           mpi_cart_shift__,
                           mpi_cart_shift_f,
                           (MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr),
                           (comm, direction, disp, rank_source, rank_dest, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/c/profile/defines.h"
#endif

void mpi_cart_shift_f(MPI_Fint *comm, MPI_Fint *direction, MPI_Fint *disp, MPI_Fint *rank_source, MPI_Fint *rank_dest, MPI_Fint *ierr)
{

}
