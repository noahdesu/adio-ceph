/*
 * $HEADER$
 */

#include "lam_config.h"

#include "mpi.h"
#include "mpi/c/bindings.h"
#include "attribute/attribute.h"

#if LAM_HAVE_WEAK_SYMBOLS && LAM_PROFILING_DEFINES
#pragma weak MPI_Type_get_attr = PMPI_Type_get_attr
#endif

#if LAM_PROFILING_DEFINES
#include "mpi/c/profile/defines.h"
#endif

static char FUNC_NAME[] = "MPI_Type_get_attr";

int
MPI_Type_get_attr (MPI_Datatype type,
                   int type_keyval,
                   void *attribute_val,
                   int *flag)
{
    int ret;

    if (MPI_PARAM_CHECK) {
	if ((NULL == attribute_val) || (NULL == flag)) {
	    return LAM_ERRHANDLER_INVOKE((lam_communicator_t *) NULL, 
					 MPI_ERR_ARG, 
					 FUNC_NAME);
	}
    }

    ret = lam_attr_get(type->d_keyhash, type_keyval, 
		       attribute_val, flag);

    LAM_ERRHANDLER_RETURN(ret, (lam_communicator_t *) NULL,
			  MPI_ERR_OTHER, FUNC_NAME);  
}
