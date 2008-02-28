/*file .c : spawned  the file Exe*/
#include <stdio.h>
#include <unistd.h>

#include "opal/util/argv.h"

#include "orte/util/proc_info.h"
#include "orte/mca/plm/plm.h"
#include "orte/mca/rml/rml.h"
#include "orte/mca/errmgr/errmgr.h"
#include "orte/runtime/runtime.h"
#include "orte/runtime/orte_globals.h"
#include "orte/util/name_fns.h"

int main(int argc, char* argv[])
{
    int rc;
    orte_job_t *jdata;
    orte_app_context_t *app;
    char cwd[1024];
    int iter;
    orte_std_cntr_t dummy;
    
    if (0 > (rc = orte_init(ORTE_NON_TOOL))) {
        fprintf(stderr, "couldn't init orte - error code %d\n", rc);
        return rc;
    }
    
    for (iter = 0; iter < 1000; ++iter) {
        /* setup the job object */
        jdata = OBJ_NEW(orte_job_t);
        
        /* create an app_context that defines the app to be run */
        app = OBJ_NEW(orte_app_context_t);
        app->app = strdup("orte_loop_child");
        opal_argv_append_nosize(&app->argv, "orte_loop_child");
        app->num_procs = 1;
        
        getcwd(cwd, sizeof(cwd));
        app->cwd = strdup(cwd);
        app->user_specified_cwd = false;
    
        /* add the app to the job data */
        orte_pointer_array_add(&dummy, jdata->apps, app);
        jdata->num_apps = 1;
        
        fprintf(stderr, "Parent: spawning child %d\n", iter);
        if (ORTE_SUCCESS != (rc = orte_plm.spawn(jdata))) {
            ORTE_ERROR_LOG(rc);
            exit(1);
        }
    }

    /* All done */
    orte_finalize();
    return 0;
}
