/*
 * $HEADER$
 */

#include "ompi_config.h"

#include <stdio.h>

#include "include/constants.h"
#include "mca/mca.h"
#include "mca/base/base.h"
#include "mca/pcmclient/pcmclient.h"
#include "mca/pcmclient/base/base.h"


int mca_pcmclient_base_close(void)
{
  /* Close all remaining available modules (may be one if this is a
     OMPI RTE program, or [possibly] multiple if this is ompi_info) */

  mca_base_components_close(mca_pcmclient_base_output, 
                            &mca_pcmclient_base_components_available, NULL);

  /* All done */

  return OMPI_SUCCESS;
}
