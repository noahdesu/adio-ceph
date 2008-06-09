/*
 * Copyright (c) 2004-2007 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "ompi_config.h"

#include "opal/mca/mca.h"
#include "opal/mca/base/mca_base_param.h"
#include "../pml_v.h"
#include "../pml_v_protocol_base.h"
#include "vprotocol_example.h"


static inline int mca_param_register_int( const char* param_name, int default_value);


static int mca_vprotocol_example_component_open(void);
static int mca_vprotocol_example_component_close(void);

static mca_pml_v_protocol_base_module_t *mca_vprotocol_example_component_init( int* priority,
                            bool, bool);
static int mca_vprotocol_example_component_finalize(void);


static int _priority;


mca_pml_v_protocol_base_component_1_0_0_t mca_vprotocol_example_component = 
{
  /* First, the mca_base_component_t struct containing meta
   * information about the component itself */
  {
    /* Indicate that we are a pml v1.0.0 component (which also implies
       a specific MCA version) */
    MCA_VPROTOCOL_BASE_VERSION_1_0_0,
    "example", /* MCA component name */
    1,  /* MCA component major version */
    0,  /* MCA component minor version */
    0,  /* MCA component release version */
    mca_vprotocol_example_component_open,  /* component open */
    mca_vprotocol_example_component_close  /* component close */
  },

  /* Next the MCA v1.0.0 component meta data */
  {
    /* Whether the component is checkpointable or not */
    false
  },

  mca_vprotocol_example_component_init,  /* component init */
  mca_vprotocol_example_component_finalize   /* component finalize */
};

/** MCA level functions
  */
  
int mca_vprotocol_example_component_open(void)
{
  _priority = mca_param_register_int( "priority", -1);
  V_OUTPUT_VERBOSE(10, "vprotocol_example_open, read priority %d", _priority);
  return OMPI_SUCCESS;
}

int mca_vprotocol_example_component_close(void)
{
  V_OUTPUT_VERBOSE(10, "vprotocol_example_close");
  return OMPI_SUCCESS;
}

/** VPROTOCOL level functions (same as PML one)
  */
  
mca_pml_v_protocol_base_module_t *mca_vprotocol_example_component_init( int* priority,
                                                                          bool enable_progress_threads,
                                                                          bool enable_mpi_threads)
{
  V_OUTPUT_VERBOSE(10, "vprotocol_example_init");
  *priority = _priority;

/**
  * Some protocols requires sanity check about thread support (those making piecewise deterministic assumption) 
  if(enable_mpi_threads)
  {
    OPAL_OUTPUT_VERBOSE( mca_pml_v_verbose, mca_pml_v_output, "vprotocol_example.init: threads are enabled, and not supported by vprotocol example fault tolerant layer, will not load"));
    return NULL;
  }
  */
  
/**
  * Insert your own protocol initialization here
  */

  return &mca_vprotocol_example.super;
}
                                                                          
int mca_vprotocol_example_component_finalize(void)
{
  V_OUTPUT_VERBOSE(10, "vprotocol_example_finalize");
  
/**
  * Insert your own garbage collecting here
  */
  
  return OMPI_SUCCESS;
}

/**
  * Helper function to set int type mca parameter
  */ 
static inline int mca_param_register_int( const char* param_name,
                                                  int default_value )
{
  int id = mca_base_param_register_int("vprotocol", "example", param_name, NULL, default_value);
  int param_value = default_value;
  mca_base_param_lookup_int(id, &param_value);
  return param_value;
}

