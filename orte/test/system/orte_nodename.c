/* -*- C -*-
 *
 * $HEADER$
 *
 * The most basic of MPI applications
 */

#include <stdio.h>
#include <unistd.h>

#include "orte/util/proc_info.h"
#include "orte/util/name_fns.h"
#include "orte/runtime/orte_globals.h"
#include "orte/runtime/runtime.h"

int main(int argc, char* argv[])
{
    int rc;
    char hostname[512];
    pid_t pid;
    
    if (0 > (rc = orte_init(ORTE_NON_TOOL))) {
        fprintf(stderr, "orte_nodename: couldn't init orte - error code %d\n", rc);
        return rc;
    }

    gethostname(hostname, 512);
    pid = getpid();

    printf("orte_nodename: Node %s %ld Name %s Pid %ld Local Rank: %ld Num_local_procs %ld\n",
           hostname, (long)orte_proc_info.nodeid, orte_util_print_name_args(ORTE_PROC_MY_NAME), (long)pid,
           (long)orte_proc_info.local_rank, (long)orte_proc_info.num_local_procs);

    orte_finalize();
    return 0;
}
