/* -*- C -*-
 * 
 * $HEADER$
 *
 */

#include "lam_config.h"

#include "mca/lam/registry/registry.h"
#include "mca/lam/registry/cofs/src/registry_cofs.h"
#include "lam/mem/malloc.h"
#include "lam/types.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

int
mca_registry_cofs_publish(char* key, void* data, size_t data_len)
{
  return LAM_ERR_NOT_IMPLEMENTED;
}


int
mca_registry_cofs_lookup(char* key, void** data, size_t* data_len)
{
  return LAM_ERR_NOT_IMPLEMENTED;
}


int
mca_registry_cofs_unpublish(char* key)
{
  return LAM_ERR_NOT_IMPLEMENTED;
}
