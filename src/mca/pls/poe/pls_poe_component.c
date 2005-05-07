/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 *
 * These symbols are in a file by themselves to provide nice linker
 * semantics.  Since linkers generally pull in symbols by object
 * files, keeping these symbols as the only symbols in this file
 * prevents utility programs such as "ompi_info" from having to import
 * entire components just to query their version and parameters.
 */

#include "ompi_config.h"

#include "include/orte_constants.h"
#include "mca/pls/pls.h"
#include "pls_poe.h"
#include "util/path.h"
#include "mca/pls/poe/pls-poe-version.h"
#include "mca/base/mca_base_param.h"


/*
 * Public string showing the pls ompi_poe component version number
 */
const char *mca_pls_poe_component_version_string =
  "Open MPI poe pls MCA component version " MCA_pls_poe_VERSION;


/*
 * Local variable
 */


/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

orte_pls_poe_component_t mca_pls_poe_component = {
    {
    /* First, the mca_component_t struct containing meta information
       about the component itself */

    {
        /* Indicate that we are a pls v1.0.0 component (which also
           implies a specific MCA version) */

        ORTE_PLS_BASE_VERSION_1_0_0,

        /* Component name and version */

        "poe",
        MCA_pls_poe_MAJOR_VERSION,
        MCA_pls_poe_MINOR_VERSION,
        MCA_pls_poe_RELEASE_VERSION,

        /* Component open and close functions */

        orte_pls_poe_component_open,
        NULL
    },

    /* Next the MCA v1.0.0 component meta data */

    {
        /* Whether the component is checkpointable or not */

        false
    },

    /* Initialization / querying functions */

    orte_pls_poe_component_init
    }
};

/**
orte_pls_poe_param_reg_int - register and lookup a integer parameter 
@param param_name parameter name [INPUT]
@param default_value default value [INPUT]
@return parameter value
*/
int orte_pls_poe_param_reg_int(char * param_name, int default_value)
{
    int id, param_value;
    id = mca_base_param_register_int("pls","poe",param_name,NULL,default_value);
    param_value = default_value;
    mca_base_param_lookup_int(id,&param_value);
    return param_value;
}

/**
orte_pls_poe_param_reg_string - register and lookup a string parameter 
@param param_name parameter name [INPUT]
@param default_value default value [INPUT]
@return parameter value
*/
char* orte_pls_rsh_param_register_string(char* param_name, char* default_value)
{
    char *param_value;
    int id;
    id = mca_base_param_register_string("pls","poe",param_name,NULL,default_value);
    mca_base_param_lookup_string(id, &param_value);
    return param_value;
}

/**
orte_pls_poe_component_open - open component and register all parameters
@return error number
*/
int orte_pls_poe_component_open(void)
{
    mca_pls_poe_component.priority = orte_pls_poe_param_reg_int("priority", 100);
    return ORTE_SUCCESS;
}

/**
orte_pls_poe_component_init - initialize component, check if we can run on this machine.
@return error number
*/
orte_pls_base_module_t *orte_pls_poe_component_init(int *priority)
{
    extern char **environ;
    mca_pls_poe_component.path = ompi_path_findv("poe", 0, environ, NULL);
    if (NULL == mca_pls_poe_component.path) {
        return NULL;
    }
    *priority = mca_pls_poe_component.priority;
    return &orte_pls_poe_module;
}
