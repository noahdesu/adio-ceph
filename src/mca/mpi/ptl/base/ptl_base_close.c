/*
 * $HEADER$
 */

#include "lam_config.h"

#include <stdio.h>

#include "lam/constants.h"
#include "mca/mca.h"
#include "mca/lam/base/base.h"
#include "mca/mpi/ptl/ptl.h"
#include "mca/mpi/ptl/base/base.h"


int mca_ptl_base_close(void)
{
  lam_list_item_t *item;
  mca_ptl_base_selected_module_t *sm;

  /* Finalize all the ptl modules and free their list items */

  for (item = lam_list_remove_first(&mca_ptl_base_modules_initialized);
       NULL != item; 
       item = lam_list_remove_first(&mca_ptl_base_modules_initialized)) {
    sm = (mca_ptl_base_selected_module_t *) item;

    /* Blatently ignore the return code (what would we do to recover,
       anyway?  This module is going away, so errors don't matter
       anymore) */

    sm->pbsm_actions->ptl_finalize(sm->pbsm_actions);
    LAM_FREE(sm);
  }

  /* Close all remaining available modules (may be one if this is a
     LAM RTE program, or [possibly] multiple if this is laminfo) */

  mca_base_modules_close(mca_ptl_base_output, 
                         &mca_ptl_base_modules_available, NULL);

  /* All done */

  return LAM_SUCCESS;
}
