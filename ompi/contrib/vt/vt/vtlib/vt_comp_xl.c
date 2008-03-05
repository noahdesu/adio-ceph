/**
 * VampirTrace
 * http://www.tu-dresden.de/zih/vampirtrace
 *
 * Copyright (c) 2005-2008, ZIH, TU Dresden, Federal Republic of Germany
 *
 * Copyright (c) 1998-2005, Forschungszentrum Juelich GmbH, Federal
 * Republic of Germany
 *
 * See the file COPYRIGHT in the package base directory for details
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vt_comp.h"
#include "vt_memhook.h"
#include "vt_pform.h"
#include "vt_trc.h"
#if (defined (VT_OMPI) || defined (VT_OMP))
#  include <omp.h>
#endif

/*
 *-----------------------------------------------------------------------------
 * Simple hash table to map function data to region identifier
 *-----------------------------------------------------------------------------
 */

typedef struct HN {
  long id;            /* hash code (address of function name) */
  uint32_t vtid;      /* associated region identifier  */
  char* func;
  char* file;
  int lno;
  struct HN* next;
} HashNode;

#define HASH_MAX 1021

static int xl_init = 1;       /* is initialization needed? */

static HashNode* htab[HASH_MAX];

/*
 * Stores region identifier `e' under hash code `h'
 */

static HashNode *hash_put(long h, uint32_t e) {
  long id = h % HASH_MAX;
  HashNode *add = (HashNode*)malloc(sizeof(HashNode));
  add->id = h;
  add->vtid = e;
  add->next = htab[id];
  htab[id] = add;
  return add;
}

/*
 * Lookup hash code `h'
 * Returns pointer to function data if already stored, otherwise 0
 */

static HashNode *hash_get(long h) {
  long id = h % HASH_MAX;
  HashNode *curr = htab[id];
  while ( curr ) {
    if ( curr->id == h ) {
      return curr;
    }
    curr = curr->next;
  }
  return 0;
}

/*
 * Register new region
 */

static HashNode *register_region(char *func, char *file, int lno) {
  uint32_t rid;
  uint32_t fid;
  HashNode* nhn;

  /* -- register file and region and store region identifier -- */
  fid = vt_def_file(file);
  rid = vt_def_region(func, fid, lno, VT_NO_LNO, VT_DEF_GROUP, VT_FUNCTION);
  nhn = hash_put((long) func, rid);
  nhn->func = func;
  nhn->file = file;
  nhn->lno  = lno;
  return nhn;
}

/*
 * This function is called at the entry of each function
 * The call is generated by the IBM xl compilers
 */

void __func_trace_enter(char* name, char *fname, int lno) {
  HashNode *hn;
  uint64_t time;

  /* -- if not yet initialized, initialize VampirTrace -- */
  if ( xl_init ) {
    VT_MEMHOOKS_OFF();
    xl_init = 0;
    vt_open();
    VT_MEMHOOKS_ON();
  }

  /* -- ignore IBM OMP runtime functions -- */
  if ( strchr(name, '@') != NULL ) return;

  VT_MEMHOOKS_OFF();

  time = vt_pform_wtime();

  /* -- get region identifier -- */
  if ( (hn = hash_get((long) name)) == 0 ) {
    /* -- region entered the first time, register region -- */
#   if defined (VT_OMPI) || defined (VT_OMP)
    if (omp_in_parallel()) {
#     pragma omp critical (vt_comp_xl_1)
      {
        if ( (hn = hash_get((long) name)) == 0 ) {
          hn = register_region(name, fname, lno);
        }
      }
    } else {
      hn = register_region(name, fname, lno);
    }
#   else
    hn = register_region(name, fname, lno);
#   endif
  }

  /* -- write enter record -- */
  vt_enter(&time, hn->vtid);

  VT_MEMHOOKS_ON();
}

/*
 * This function is called at the exit of each function
 * The call is generated by the IBM xl compilers
 */

void __func_trace_exit(char* name, char *fname, int lno) {
  HashNode *hn;
  uint64_t time;

  VT_MEMHOOKS_OFF();

  /* -- ignore IBM OMP runtime functions -- */
  if ( strchr(name, '@') != NULL )
  {
    VT_MEMHOOKS_ON();
    return;
  }

  time = vt_pform_wtime();

  /* -- write exit record -- */
  hn = hash_get((long) name);

  vt_exit(&time);

  VT_MEMHOOKS_ON();
}
