/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "mpi.h"
#include "mpi/f77/bindings.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak PMPI_FILE_CLOSE = mpi_file_close_f
#pragma weak pmpi_file_close = mpi_file_close_f
#pragma weak pmpi_file_close_ = mpi_file_close_f
#pragma weak pmpi_file_close__ = mpi_file_close_f
#elif OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (PMPI_FILE_CLOSE,
                           pmpi_file_close,
                           pmpi_file_close_,
                           pmpi_file_close__,
                           pmpi_file_close_f,
                           (MPI_Fint *fh, MPI_Fint *ierr),
                           (fh, ierr) )
#endif

#if OMPI_HAVE_WEAK_SYMBOLS
#pragma weak MPI_FILE_CLOSE = mpi_file_close_f
#pragma weak mpi_file_close = mpi_file_close_f
#pragma weak mpi_file_close_ = mpi_file_close_f
#pragma weak mpi_file_close__ = mpi_file_close_f
#endif

#if ! OMPI_HAVE_WEAK_SYMBOLS && ! OMPI_PROFILE_LAYER
OMPI_GENERATE_F77_BINDINGS (MPI_FILE_CLOSE,
                           mpi_file_close,
                           mpi_file_close_,
                           mpi_file_close__,
                           mpi_file_close_f,
                           (MPI_Fint *fh, MPI_Fint *ierr),
                           (fh, ierr) )
#endif


#if OMPI_PROFILE_LAYER && ! OMPI_HAVE_WEAK_SYMBOLS
#include "mpi/f77/profile/defines.h"
#endif

OMPI_EXPORT
void mpi_file_close_f(MPI_Fint *fh, MPI_Fint *ierr)
{
    MPI_File c_fh;

    c_fh = MPI_File_f2c(*fh);

    *ierr = OMPI_INT_2_FINT(MPI_File_close(&c_fh));
    *fh = MPI_File_c2f(c_fh);
}
