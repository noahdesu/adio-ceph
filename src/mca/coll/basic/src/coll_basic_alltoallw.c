/*
 * $HEADER$
 */

#include "ompi_config.h"
#include "coll_basic.h"

#include "mpi.h"
#include "include/constants.h"
#include "datatype/datatype.h"
#include "communicator/communicator.h"
#include "mca/coll/coll.h"
#include "mca/coll/base/coll_tags.h"


/*
 *	alltoallw_intra
 *
 *	Function:	- MPI_Alltoallw for non-ompid RPIs
 *	Accepts:	- same as MPI_Alltoallw()
 *	Returns:	- MPI_SUCCESS or an MPI error code
 */
int mca_coll_basic_alltoallw_intra(void *sbuf, int *scounts, int *sdisps,
                                   struct ompi_datatype_t **sdtypes, 
                                   void *rbuf, int *rcounts, int *rdisps,
                                   struct ompi_datatype_t **rdtypes, 
                                   struct ompi_communicator_t *comm)
{
  int i;
  int size;
  int rank;
  int err;
  char *psnd;
  char *prcv;
  size_t nreqs;
  MPI_Request *preq;

  /* Initialize. */

  size = ompi_comm_size(comm);
  rank = ompi_comm_rank(comm);

  /* simple optimization */

  psnd = ((char *) sbuf) + sdisps[rank];
  prcv = ((char *) rbuf) + rdisps[rank];

  err = ompi_ddt_sndrcv(psnd, scounts[rank], sdtypes[rank],
                       prcv, rcounts[rank], rdtypes[rank], 
                       MCA_COLL_BASE_TAG_ALLTOALLW, comm);
  if (MPI_SUCCESS != err) {
    return err;
  }

  /* If only one process, we're done. */

  if (1 == size) {
    return MPI_SUCCESS;
  }

  /* Initiate all send/recv to/from others. */

  nreqs = (size - 1) * 2;
  preq = comm->c_coll_basic_data->mccb_reqs;

  /* Post all receives first -- a simple optimization */

  for (i = 0; i < size; ++i) {
    if (i == rank)
      continue;

    prcv = ((char *) rbuf) + rdisps[i];
    err = mca_pml.pml_irecv_init(prcv, rcounts[i], rdtypes[i],
                                 i, MCA_COLL_BASE_TAG_ALLTOALLW, comm, preq++);
    if (MPI_SUCCESS != err) {
      mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);
      return err;
    }
  }

  /* Now post all sends */

  for (i = 0; i < size; ++i) {
    if (i == rank)
      continue;

    psnd = ((char *) sbuf) + sdisps[i];
    err = mca_pml.pml_isend_init(psnd, scounts[i], sdtypes[i],
                                 i, MCA_COLL_BASE_TAG_ALLTOALLW, 
                                 MCA_PML_BASE_SEND_STANDARD, comm, preq++);
    if (MPI_SUCCESS != err) {
      mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);
      return err;
    }
  }

  /* Start your engines.  This will never return an error. */

  mca_pml.pml_start(nreqs, comm->c_coll_basic_data->mccb_reqs);

  /* Wait for them all.  If there's an error, note that we don't care
     what the error was -- just that there *was* an error.  The PML
     will finish all requests, even if one or more of them fail.
     i.e., by the end of this call, all the requests are free-able.
     So free them anyway -- even if there was an error, and return the
     error after we free everything. */

  err = mca_pml.pml_wait_all(nreqs, comm->c_coll_basic_data->mccb_reqs,
                             MPI_STATUSES_IGNORE);

  /* Free the requests. */

  mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);

  /* All done */

  return MPI_SUCCESS;
}


/*
 *	alltoallw_inter
 *
 *	Function:	- MPI_Alltoallw for non-lamd RPIs
 *	Accepts:	- same as MPI_Alltoallw()
 *	Returns:	- MPI_SUCCESS or an MPI error code
 */
int mca_coll_basic_alltoallw_inter(void *sbuf, int *scounts, int *sdisps,
                                   struct ompi_datatype_t **sdtypes,
                                   void *rbuf, int *rcounts, int *rdisps,
                                   struct ompi_datatype_t **rdtypes,
                                   struct ompi_communicator_t *comm)
{
    int i;
    int size;
    int rank;
    int err;
    char *psnd;
    char *prcv;
    size_t nreqs;
    MPI_Request *preq;
    
    /* Initialize. */
    size = ompi_comm_remote_size(comm);
    rank = ompi_comm_rank(comm);
    
    /* Initiate all send/recv to/from others. */
    nreqs = size * 2;
    preq = comm->c_coll_basic_data->mccb_reqs;
    
    /* Post all receives first -- a simple optimization */
    for (i = 0; i < size; ++i) {
        prcv = ((char *) rbuf) + rdisps[i];
        err = mca_pml.pml_irecv_init(prcv, rcounts[i], rdtypes[i],
                                     i, MCA_COLL_BASE_TAG_ALLTOALLW, 
                                     comm, preq++);
        if (OMPI_SUCCESS != err) {
            mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);
            return err;
        }
    }
  
    /* Now post all sends */
    for (i = 0; i < size; ++i) {
        psnd = ((char *) sbuf) + sdisps[i];
        err = mca_pml.pml_isend_init(psnd, scounts[i], sdtypes[i],
                                     i, MCA_COLL_BASE_TAG_ALLTOALLW, 
                                     MCA_PML_BASE_SEND_STANDARD, comm, preq++);
        if (OMPI_SUCCESS != err) {
            mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);
            return err;
        }
    }
    
    /* Start your engines.  This will never return an error. */
    mca_pml.pml_start(nreqs, comm->c_coll_basic_data->mccb_reqs);
    
    /* Wait for them all.  If there's an error, note that we don't care
       what the error was -- just that there *was* an error.  The PML
       will finish all requests, even if one or more of them fail.
       i.e., by the end of this call, all the requests are free-able.
       So free them anyway -- even if there was an error, and return the
       error after we free everything. */
    err = mca_pml.pml_wait_all(nreqs, comm->c_coll_basic_data->mccb_reqs,
                               MPI_STATUSES_IGNORE);
    
    /* Free the requests. */
    mca_coll_basic_free_reqs(comm->c_coll_basic_data->mccb_reqs, nreqs);
    
    /* All done */
    return err;
}
