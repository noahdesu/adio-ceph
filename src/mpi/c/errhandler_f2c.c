/*
 * $HEADER$
 */

#include "ompi_config.h"

#include "mpi.h"
#include "mpi/c/bindings.h"
#include "errhandler/errhandler.h"
#include "communicator/communicator.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILING_DEFINES
#pragma weak MPI_Errhandler_f2c = PMPI_Errhandler_f2c
#endif

#if OMPI_PROFILING_DEFINES
#include "mpi/c/profile/defines.h"
#endif


MPI_Errhandler MPI_Errhandler_f2c(MPI_Fint errhandler_f)
{
  size_t eh_index = (size_t) errhandler_f;

  /* Error checking */

  if (MPI_PARAM_CHECK) {
    if (0 > eh_index || 
        eh_index >= ompi_pointer_array_get_size(ompi_errhandler_f_to_c_table)) {
      return MPI_ERRHANDLER_NULL;
    }
  }

  /* All done */

  return ompi_errhandler_f_to_c_table->addr[eh_index];
}
