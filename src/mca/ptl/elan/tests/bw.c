#include "mpi.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "test_util.h"

#define MYBUFSIZE (4*1024*1024)
#define MAX_REQ_NUM 1024

MPI_Request request[MAX_REQ_NUM];
MPI_Status  stat[MAX_REQ_NUM];

int         skip = 0;

int
main (int argc,
      char *argv[])
{
    int         myid, numprocs, i;
    int         size, loop, win, page_size;
    struct timeval t_start, t_end;
    char        s_buf[MYBUFSIZE];
    char        r_buf[MYBUFSIZE];

    /* Get some environmental variables set for Open MPI, OOB */
    env_init_for_elan();

    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);

    if (argc < 3) {
        fprintf (stderr, "Usage: %s loop win size\n", argv[0]);
        MPI_Finalize ();
        return 0;
    }
    size= atoi (argv[3]);
    win = atoi (argv[2]);
    loop= atoi (argv[1]);
    page_size = getpagesize ();

    for (i = 0; i < size; i++) {
        s_buf[i] = 'a';
        r_buf[i] = 'b';
    }

    MPI_Barrier (MPI_COMM_WORLD);

    for (i = 0; i < loop + skip; i++) {
	if (myid == 0) {
	    /* Start time */
	    if ( i == skip )
		gettimeofday (&t_start, 0);

	    for (i = 0; i < win; i++) {
		MPI_Isend (s_buf, size, MPI_CHAR, 1, 100, MPI_COMM_WORLD,
			   request + i);
	    }
	    MPI_Waitall (win, request, stat);
	    MPI_Recv (r_buf, 4, MPI_CHAR, 1, 101, MPI_COMM_WORLD, &stat[0]);
	} else {
	    for (i = 0; i < win; i++) {
		MPI_Irecv (r_buf, size, MPI_CHAR, 0, 100, MPI_COMM_WORLD,
			   request + i);
	    }
	    MPI_Waitall (win, request, stat);
	    MPI_Send (s_buf, 4, MPI_CHAR, 0, 101, MPI_COMM_WORLD);
	}
	MPI_Barrier (MPI_COMM_WORLD);
    }

    if (myid == 0) {
        double      latency;
	gettimeofday (&t_end, 0);
	latency = ((1.0e6 * t_end.tv_sec + t_end.tv_usec) 
		- (1.0e6 * t_start.tv_sec + t_start.tv_usec)) 
	    / (loop * win);
        fprintf (stdout, "%8d  %8.2f\n", size, size / latency);
        fflush (stdout);
    }

    MPI_Finalize ();
    return 0;
}
