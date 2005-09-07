/* @file */
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
 */

#ifndef OPAL_TRACE_H_
#define OPAL_TRACE_H_

#include "ompi_config.h"

#ifndef ENABLE_TRACE
#define ENABLE_TRACE 0
#endif

#if ENABLE_TRACE

#include "opal/util/output.h"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

#define OPAL_TRACE()
    do {                                                    \
        opal_output(opal_trace_handle, "TRACE: %s @ %s:%d",   \
            __func__, __FILE__, __LINE__);                  \
       } while (0)
#else

#define OPAL_TRACE()

#endif /* ENABLE_TRACE */

extern int opal_trace_handle;

OMPI_DECLSPEC void opal_trace_init(void);
OMPI_DECLSPEC void opal_trace_finalize(void);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* OPAL_TRACE_H */
