/* -*- C -*-
 *
 * $HEADER$
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ompi_config.h"

#include "include/types.h"
#include "mca/oob/oob.h"
#include "mca/oob/cofs/src/oob_cofs.h"
#include "mca/ns/base/base.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

static int do_recv(mca_ns_base_jobid_t jobid, mca_ns_base_vpid_t procid, const struct iovec* iov, int count, int flags);

/**
*  Similiar to unix send(2).
*
* @param peer (IN)   Opaque name of peer process.
* @param msg (IN)    Array of iovecs describing user buffers and lengths.
* @param count (IN)  Number of elements in iovec array.
* @param flags (IN)  Currently unused.
* @return            OMPI error code (<0) on error number of bytes actually sent.
*/

int mca_oob_cofs_send(
    const ompi_process_name_t* peer, 
    const struct iovec *iov, 
    int count, 
    int flags)
{
  FILE *fp;
  int i, wlen;
  size_t size = 0;
  char msg_file[OMPI_PATH_MAX];
  char msg_file_tmp[OMPI_PATH_MAX];

  /* create the file and open it... */
  snprintf(msg_file, OMPI_PATH_MAX, "%s/%d_%d_%d_%ld.msg", mca_oob_cofs_comm_loc,
           ompi_name_server.get_jobid(&mca_oob_base_self),
           ompi_name_server.get_vpid(&mca_oob_base_self),
           ompi_name_server.get_vpid(peer), (long)mca_oob_cofs_serial);
  snprintf(msg_file_tmp, OMPI_PATH_MAX, "%s/.%d_%d_%d_%ld.msg", mca_oob_cofs_comm_loc,
           ompi_name_server.get_jobid(&mca_oob_base_self), 
           ompi_name_server.get_vpid(&mca_oob_base_self), 
           ompi_name_server.get_vpid(peer), (long)mca_oob_cofs_serial);

  fp = fopen(msg_file_tmp, "w");
  if (fp == NULL) {
    return OMPI_ERR_OUT_OF_RESOURCE;
  }

  /* write size */
  for(i=0; i<count; i++)
      size += iov[i].iov_len;
  wlen = fwrite(&size, sizeof(size), 1, fp);
  if (wlen != 1) {
    fclose(fp);
    unlink(msg_file_tmp);
    return OMPI_ERR_OUT_OF_RESOURCE;
  }
  fflush(fp);

  /* write msg */
  wlen = writev(fileno(fp), iov, count);
  if (wlen != size) {
    fclose(fp);
    unlink(msg_file_tmp);
    return OMPI_ERROR;
  }

  /* publish the thing... */
  fclose(fp);
  rename(msg_file_tmp, msg_file);
  mca_oob_cofs_serial++;
  return wlen;
}


int mca_oob_cofs_send_nb(
    const ompi_process_name_t* peer, 
    const struct iovec *iov, 
    int count, 
    int flags,
    mca_oob_callback_fn_t cbfunc, 
    void* cbdata)
{
    int status = mca_oob_cofs_send(peer, iov, count, flags);
    if(NULL != cbfunc)
        cbfunc(status, peer, iov, count, cbdata);
    return status;
}


int
mca_oob_cofs_recv(ompi_process_name_t* peer, const struct iovec* iov, int count, int flags)
{
  int ret = OMPI_ERR_WOULD_BLOCK;
  while (ret == OMPI_ERR_WOULD_BLOCK) {
    ret = do_recv(ompi_name_server.get_jobid(peer),
                  ompi_name_server.get_vpid(peer), iov, count, flags);
    sleep(1);
  }
  return ret;
}


int
mca_oob_cofs_recv_nb(
   ompi_process_name_t* peer, 
   const struct iovec* iov, 
   int count, 
   int flags,
   mca_oob_callback_fn_t cbfunc, 
   void* cbdata)
{
   int status = mca_oob_cofs_recv(peer, iov, count, flags);
   if(NULL != cbfunc)
       cbfunc(status, peer, iov, count, cbdata);
   return status;
}


static char*
find_match(mca_ns_base_jobid_t jobid, mca_ns_base_vpid_t procid)
{
  DIR* dir;
  struct dirent *ent;
  unsigned long tmp_serial;
  int tmp_jobid, tmp_procid, tmp_myprocid;
  int ret;
  bool found = false;
  char best_name[OMPI_PATH_MAX];
  uint64_t best_serial = ((1ULL << 63) - 1);

  dir = opendir(mca_oob_cofs_comm_loc);
  if (dir == NULL) {
    return NULL;
  }

  while ((ent = readdir(dir)) != NULL) {
    if (ent->d_name[0] == '.') continue;

    ret = sscanf(ent->d_name, "%d_%d_%d_%lu.msg", &tmp_jobid, &tmp_procid, 
                 &tmp_myprocid, &tmp_serial);
    if (ret != 4) {
      continue;
    }
    
    if (tmp_jobid != jobid) {
      continue;
    }
    if (tmp_myprocid != ompi_name_server.get_vpid(&mca_oob_base_self)) {
      continue;
    }
    if (tmp_procid != procid) {
      continue;
    }

    /* do best one here... */
    found = true;
    if (tmp_serial < best_serial) {
      strcpy(best_name, ent->d_name);
      best_serial = tmp_serial;
    }
  }

  closedir(dir);
  if (found) {
    return strdup(best_name);
  } else {
    return NULL;
  }
}


static int 
do_recv(mca_ns_base_jobid_t jobid, mca_ns_base_vpid_t procid, const struct iovec* iov, int count, int flags)
{
  char *fname;
  char full_fname[OMPI_PATH_MAX];
  int fd;
  size_t rlen;
  size_t size;

  fname = find_match(jobid, procid);
  if (fname == NULL) {
    return OMPI_ERR_WOULD_BLOCK;
  }
  snprintf(full_fname, OMPI_PATH_MAX, "%s/%s", mca_oob_cofs_comm_loc, fname);
  free(fname);

  fd = open(full_fname, O_RDONLY);
  if (fd < 0) {
    return OMPI_ERROR;
  }
  if((flags & MCA_OOB_PEEK) == 0)
      unlink(full_fname);

  rlen = read(fd, &size, sizeof(size));
  if (rlen != sizeof(size)) {
    close(fd);
    return OMPI_ERROR;
  }

  if(iov != NULL && count > 0) {
      rlen = readv(fd, iov, count);
  }
  close(fd);
  return (flags & MCA_OOB_TRUNC) ? size : rlen;
}

