/*
 * $HEADER$
 */

/** @file **/

#include "ompi_config.h"

#include "include/constants.h"
#include "runtime/runtime.h"
#include "util/output.h"
#include "threads/mutex.h"
#include "mca/llm/base/base.h"
#include "mca/pcm/base/base.h"
#include "mca/pcmclient/base/base.h"
#include "mca/oob/oob.h"
#include "mca/ns/base/base.h"
#include "util/session_dir.h"

/**
 * Leave a OMPI RTE.
 *
 * @retval OMPI_SUCCESS Upon success.
 * @retval OMPI_ERROR Upon failure.
 *
 * This function performs 
 */
int ompi_rte_finalize(void)
{
  mca_oob_base_close();
  mca_pcm_base_close();
  mca_llm_base_close();
  mca_pcmclient_base_close();
  mca_ns_base_close();

  ompi_session_dir_finalize();

  /* All done */

  return OMPI_SUCCESS;
}
