/*
 * $HEADER$
 */

#include "ompi_config.h"
#include "coll_basic.h"

#include "mpi.h"
#include "include/constants.h"
#include "communicator/communicator.h"
#include "datatype/datatype.h"
#include "op/op.h"
#include "mca/coll/coll.h"
#include "mca/coll/base/coll_tags.h"
#include "coll_basic.h"


/*
 *	allreduce_intra
 *
 *	Function:	- allreduce using other MPI collectives
 *	Accepts:	- same as MPI_Allreduce()
 *	Returns:	- MPI_SUCCESS or error code
 */
int mca_coll_basic_allreduce_intra(void *sbuf, void *rbuf, int count,
                                   struct ompi_datatype_t *dtype, 
                                   struct ompi_op_t *op,
                                   struct ompi_communicator_t *comm)
{
  int err;

  /* Reduce to 0 and broadcast. */

  err = comm->c_coll.coll_reduce(sbuf, rbuf, count, dtype, op, 0, comm);
  if (MPI_SUCCESS != err) {
    return err;
  }

  return comm->c_coll.coll_bcast(rbuf, count, dtype, 0, comm);
}


/*
 *	allreduce_inter
 *
 *	Function:	- allreduce using other MPI collectives
 *	Accepts:	- same as MPI_Allreduce()
 *	Returns:	- MPI_SUCCESS or error code
 */
int mca_coll_basic_allreduce_inter(void *sbuf, void *rbuf, int count,
                                   struct ompi_datatype_t *dtype,
                                   struct ompi_op_t *op,
                                   struct ompi_communicator_t *comm)
{
    int err, i;
    int rank;
    int root=0;
    int rsize;
    long lb, extent;
    char *tmpbuf=NULL, *pml_buffer=NULL;
    ompi_request_t *req[2];
    ompi_request_t **reqs=comm->c_coll_basic_data->mccb_reqs;

    rank = ompi_comm_rank ( comm );
    rsize = ompi_comm_remote_size (comm);

    /* determine result of the remote group, you cannot
       use coll_reduce for inter-communicators, since than
       you would need to determine an order between the
       two groups (e.g. which group is providing the data
       and which one enters coll_reduce with providing
       MPI_PROC_NULL as root argument etc.) Here,
       we execute the data exchange for both groups
       simultaniously. */
    /*****************************************************************/
    if ( rank == root ) {
        err = ompi_ddt_get_extent(dtype, &lb, &extent);
        if (OMPI_SUCCESS != err) {
            return OMPI_ERROR;
        }
        
        tmpbuf = (char *)malloc (count * extent);
        if ( NULL == tmpbuf ) {
            return OMPI_ERR_OUT_OF_RESOURCE;
        }
        pml_buffer = tmpbuf - lb;
        
        /* Do a send-recv between the two root procs. to avoid deadlock */
        err = mca_pml.pml_irecv(rbuf, count, dtype, 0,
                                MCA_COLL_BASE_TAG_ALLREDUCE, comm, 
                                &(req[0]));
        if (OMPI_SUCCESS != err) {
            goto exit;
        }

        err = mca_pml.pml_isend (sbuf, count, dtype, 0, 
                                 MCA_COLL_BASE_TAG_ALLREDUCE,
                                 MCA_PML_BASE_SEND_STANDARD, 
                                 comm, &(req[1]) );
        if ( OMPI_SUCCESS != err ) {
            goto exit;
        }

        err = ompi_request_wait_all(2, req, MPI_STATUSES_IGNORE);
        if (OMPI_SUCCESS != err ) {
            goto exit;
        }

        
        /* Loop receiving and calling reduction function (C or Fortran). */
        for (i = 1; i < rsize; i++) {
            err = mca_pml.pml_recv(pml_buffer, count, dtype, i, 
                                   MCA_COLL_BASE_TAG_ALLREDUCE, comm, 
                                   MPI_STATUS_IGNORE);
            if (MPI_SUCCESS != err) {
                goto exit;
          }
            
            /* Perform the reduction */
            ompi_op_reduce(op, pml_buffer, rbuf, count, dtype);
        }
    }    
    else {
        /* If not root, send data to the root. */
        err = mca_pml.pml_send(sbuf, count, dtype, root, 
                               MCA_COLL_BASE_TAG_ALLREDUCE, 
                               MCA_PML_BASE_SEND_STANDARD, comm);
        if ( OMPI_SUCCESS != err ) {
            goto exit;
        }
    }
    

    /* now we have on one process the result of the remote group. To distribute
       the data to all processes in the local group, we exchange the data between
       the two root processes. They then send it to every other process in the 
       remote group. */
    /***************************************************************************/
    if ( rank == root ) {
        /* sendrecv between the two roots */
        err = mca_pml.pml_irecv (pml_buffer, count, dtype, 0, 
                                 MCA_COLL_BASE_TAG_ALLREDUCE,
                                 comm, &(req[1]));
        if ( OMPI_SUCCESS != err ) {
            goto exit;
        }
        
        err = mca_pml.pml_isend (rbuf, count, dtype, 0, 
                                MCA_COLL_BASE_TAG_ALLREDUCE,
                                MCA_PML_BASE_SEND_STANDARD, comm,
                                 &(req[0]));
        if ( OMPI_SUCCESS != err ) {
            goto exit;
        }
        err = ompi_request_wait_all (2, req, MPI_STATUSES_IGNORE);
        if ( OMPI_SUCCESS != err ) {
            goto exit;
        }
        
        /* distribute the data to other processes in remote group.
           Note that we start from 1 (not from zero), since zero
           has already the correct data AND we avoid a potential 
           deadlock here. 
        */
        if (rsize > 1) {
            for ( i=1; i<rsize; i++ ) {
                err = mca_pml.pml_isend (pml_buffer, count, dtype,i,
                                         MCA_COLL_BASE_TAG_ALLREDUCE,
                                         MCA_PML_BASE_SEND_STANDARD, comm,
                                         &reqs[i - 1]);
                if ( OMPI_SUCCESS != err ) {
                    goto exit;
                }
            }

            err = ompi_request_wait_all (rsize - 1, reqs, MPI_STATUSES_IGNORE);
            if ( OMPI_SUCCESS != err ) {
                goto exit;
            }
        }
    }
    else {
        err = mca_pml.pml_recv (rbuf, count, dtype, root, 
                                MCA_COLL_BASE_TAG_ALLREDUCE,
                                comm, MPI_STATUS_IGNORE);
    }

 exit:
    if ( NULL != tmpbuf ) {
        free ( tmpbuf );
    }


    return err;
}
