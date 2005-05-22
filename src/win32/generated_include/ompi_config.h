/* include/ompi_config.h.  Generated by configure.  */
/* include/ompi_config.h.in.  Generated from configure.ac by autoheader.  */

/* -*- c -*-
 *
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
 * Function: - OS, CPU and compiler dependent configuration
 */

#ifndef OMPI_CONFIG_H
#define OMPI_CONFIG_H

/* Define to 1 if you have the <aio.h> header file. */
/* #undef HAVE_AIO_H */

/* Define to 1 if you have the <alloca.h> header file. */
/* #undef HAVE_ALLOCA_H */

/* Define to 1 if you have the <arpa/inet.h> header file. */
/* #undef HAVE_ARPA_INET_H */

/* Define to 1 if you have the `asprintf' function. */
/* #undef HAVE_ASPRINTF */

/* Define to 1 if you have the <dirent.h> header file. */
/* #undef HAVE_DIRENT_H */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define if your system supports the epoll system calls */
/* #undef HAVE_EPOLL */

/* Define to 1 if you have the `epoll_ctl' function. */
/* #undef HAVE_EPOLL_CTL */

/* Define to 1 if you have the `err' function. */
/* #undef HAVE_ERR */

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define to 1 if the system has the type `int16_t'. */
/* #undef HAVE_INT16_T */

/* Define to 1 if the system has the type `int32_t'. */
/* #undef HAVE_INT32_T */

/* Define to 1 if the system has the type `int64_t'. */
/* #undef HAVE_INT64_T */

/* Define to 1 if the system has the type `int8_t'. */
/* #undef HAVE_INT8_T */

/* Define to 1 if the system has the type `intptr_t'. */
#define HAVE_INTPTR_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `kqueue' function. */
/* #undef HAVE_KQUEUE */

/* Define to 1 if you have the <libgen.h> header file. */
/* #undef HAVE_LIBGEN_H */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if the system has the type `long double'. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if the system has the type `long long'. */
#define HAVE_LONG_LONG 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <netdb.h> header file. */
/* #undef HAVE_NETDB_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define to 1 if you have the <netinet/tcp.h> header file. */
/* #undef HAVE_NETINET_TCP_H */

/* Define to 1 if you have the <net/if.h> header file. */
/* #undef HAVE_NET_IF_H */

/* Define to 1 if you have the `poll' function. */
/* #undef HAVE_POLL */

/* Define to 1 if you have the <poll.h> header file. */
/* #undef HAVE_POLL_H */

/* Define to 1 if you have the <pthread.h> header file. */
/* #undef HAVE_PTHREAD_H */

/* Define to 1 if you have the <pwd.h> header file. */
/* #undef HAVE_PWD_H */

/* Define if your system supports POSIX realtime signals */
/* #undef HAVE_RTSIG */

/* Define to 1 if you have the <sched.h> header file. */
/* #undef HAVE_SCHED_H */

/* Define to 1 if you have the `select' function. */
/* #undef HAVE_SELECT */
#define HAVE_SELECT 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `sigtimedwait' function. */
/* #undef HAVE_SIGTIMEDWAIT */

/* Define to 1 if you have the `snprintf' function. */
/* #undef HAVE_SNPRINTF */

/* Define to 1 if you have the <stdbool.h> header file. */
/* #undef HAVE_STDBOOL_H */

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if you have the <syslog.h> header file. */
/* #undef HAVE_SYSLOG_H */

/* Define to 1 if you have the <sys/epoll.h> header file. */
/* #undef HAVE_SYS_EPOLL_H */

/* Define to 1 if you have the <sys/event.h> header file. */
/* #undef HAVE_SYS_EVENT_H */

/* Define to 1 if you have the <sys/ipc.h> header file. */
/* #undef HAVE_SYS_IPC_H */

/* Define to 1 if you have the <sys/mman.h> header file. */
/* #undef HAVE_SYS_MMAN_H */

/* Define to 1 if you have the <sys/queue.h> header file. */
/* #undef HAVE_SYS_QUEUE_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
/* #undef HAVE_SYS_RESOURCE_H */

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/statvfs.h> header file. */
/* #undef HAVE_SYS_STATVFS_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
/* #undef HAVE_SYS_UIO_H */

/* Define to 1 if you have the <sys/utsname.h> header file. */
/* #undef HAVE_SYS_UTSNAME_H */

/* Define to 1 if you have the <sys/wait.h> header file. */
/* #undef HAVE_SYS_WAIT_H */

/* Define if TAILQ_FOREACH is defined in <sys/queue.h> */
/* #undef HAVE_TAILQFOREACH */

/* Define to 1 if you have the <termios.h> header file. */
/* #undef HAVE_TERMIOS_H */

/* Define if timeradd is defined in <sys/time.h> */
/* #undef HAVE_TIMERADD */

/* Define to 1 if the system has the type `uint16_t'. */
/* #undef HAVE_UINT16_T */

/* Define to 1 if the system has the type `uint32_t'. */
/* #undef HAVE_UINT32_T */

/* Define to 1 if the system has the type `uint64_t'. */
/* #undef HAVE_UINT64_T */

/* Define to 1 if the system has the type `uint8_t'. */
/* #undef HAVE_UINT8_T */

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the <ulimit.h> header file. */
/* #undef HAVE_ULIMIT_H */

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the `vasprintf' function. */
/* #undef HAVE_VASPRINTF */

/* Define to 1 if you have the `vsnprintf' function. */
/* #undef HAVE_VSNPRINTF */

/* Define if kqueue works correctly with pipes */
/* #undef HAVE_WORKING_KQUEUE */

/* Define if realtime signals work on pipes */
/* #undef HAVE_WORKING_RTSIG */

/* C type corresponding to Fortran INTEGER */
#define MPI_Fint int

/* Type of MPI_Offset */
#define MPI_Offset long long

/* Whether we want to check MPI parameters always, never, or decide at
   run-time */
#define MPI_PARAM_CHECK ompi_mpi_param_check

/* Alignment of type char */
#define OMPI_ALIGNMENT_CHAR 1

/* Alignment of type double */
#define OMPI_ALIGNMENT_DOUBLE 8

/* Alignment of type float */
#define OMPI_ALIGNMENT_FLOAT 4

/* Alignment of fortran complex */
#define OMPI_ALIGNMENT_FORTRAN_COMPLEX 0

/* Alignment of fortran double complex */
#define OMPI_ALIGNMENT_FORTRAN_DBLCOMPLEX 0

/* Alignment of fortran double precision */
#define OMPI_ALIGNMENT_FORTRAN_DBLPREC 0

/* Alignment of fortran integer */
#define OMPI_ALIGNMENT_FORTRAN_INT 0

/* Alignment of fortran logical */
#define OMPI_ALIGNMENT_FORTRAN_LOGICAL 

/* alignment of fortran real */
#define OMPI_ALIGNMENT_FORTRAN_REAL 0

/* Alignment of type int */
#define OMPI_ALIGNMENT_INT 4

/* Alignment of type long */
#define OMPI_ALIGNMENT_LONG 4

/* Alignment of type long double */
#define OMPI_ALIGNMENT_LONG_DOUBLE 8

/* Alignment of type long long */
#define OMPI_ALIGNMENT_LONG_LONG 8

/* Alignment of type short */
#define OMPI_ALIGNMENT_SHORT 2

/* Alignment of type void * */
#define OMPI_ALIGNMENT_VOID_P 4

/* Alignment of type wchar_t */
#define OMPI_ALIGNMENT_WCHAR 2

/* OMPI architecture string */
#define OMPI_ARCH "i686-pc-cygwin"

/* OMPI underlying C compiler */
#define OMPI_CC "cl"

/* OMPI underlying C++ compiler */
#define OMPI_CXX "cl"

/* Whether we want developer-level debugging code or not */
#define OMPI_ENABLE_DEBUG 1

/* Whether we want the memory profiling or not */
#define OMPI_ENABLE_MEM_DEBUG 1

/* Whether we want the memory profiling or not */
#define OMPI_ENABLE_MEM_PROFILE 1

/* Whether we want MPI profiling or not */
#define OMPI_ENABLE_MPI_PROFILING 1

/* Do we want to use the event library signal handlers */
#define OMPI_EVENT_USE_SIGNALS 1

/* OMPI underlying F77 compiler */
#define OMPI_F77 "g77"

/* Whether fortran symbols are all caps or not */
#define OMPI_F77_CAPS 0

/* Whether fortran symbols have a trailing double underscore or not */
#define OMPI_F77_DOUBLE_UNDERSCORE 0

/* Whether fortran symbols have no trailing underscore or not */
#define OMPI_F77_PLAIN 0

/* Whether fortran symbols have a trailing underscore or not */
#define OMPI_F77_SINGLE_UNDERSCORE 0

/* Whether or not we have compiled with C++ exceptions support */
#define OMPI_HAVE_CXX_EXCEPTION_SUPPORT 0

/* Whether we have FORTRAN COMPLEX16 or not */
#define OMPI_HAVE_FORTRAN_COMPLEX16 0

/* Whether we have FORTRAN COMPLEX32 or not */
#define OMPI_HAVE_FORTRAN_COMPLEX32 0

/* Whether we have FORTRAN COMPLEX8 or not */
#define OMPI_HAVE_FORTRAN_COMPLEX8 0

/* Whether we have FORTRAN INTEGER1 or not */
#define OMPI_HAVE_FORTRAN_INTEGER1 0

/* Whether we have FORTRAN INTEGER16 or not */
#define OMPI_HAVE_FORTRAN_INTEGER16 0

/* Whether we have FORTRAN INTEGER2 or not */
#define OMPI_HAVE_FORTRAN_INTEGER2 0

/* Whether we have FORTRAN INTEGER4 or not */
#define OMPI_HAVE_FORTRAN_INTEGER4 0

/* Whether we have FORTRAN INTEGER8 or not */
#define OMPI_HAVE_FORTRAN_INTEGER8 0

/* Whether we have FORTRAN REAL16 or not */
#define OMPI_HAVE_FORTRAN_REAL16 0

/* Whether we have FORTRAN REAL4 or not */
#define OMPI_HAVE_FORTRAN_REAL4 0

/* Whether we have FORTRAN REAL8 or not */
#define OMPI_HAVE_FORTRAN_REAL8 0

/* Do we have POSIX threads */
#define OMPI_HAVE_POSIX_THREADS 0

/* Do we have native Solaris threads */
#define OMPI_HAVE_SOLARIS_THREADS 0

/* Do we want to enable MPI Threads */
#define OMPI_ENABLE_MPI_THREADS 0

/* Do we want the progress thread */
#define OMPI_ENABLE_PROGRESS_THREADS 0

/* Whether we have __va_copy or not */
#define OMPI_HAVE_UNDERSCORE_VA_COPY 0

/* Whether we have va_copy or not */
#define OMPI_HAVE_VA_COPY 0

/* Wehther we have weak symbols or not */
#define OMPI_HAVE_WEAK_SYMBOLS 0

/* Size of fortran complex */
#define OMPI_SIZEOF_FORTRAN_COMPLEX 0

/* Size of fortran double complex */
#define OMPI_SIZEOF_FORTRAN_DBLCOMPLEX 0

/* Size of fortran double precision */
#define OMPI_SIZEOF_FORTRAN_DBLPREC 8

/* Size of fortran integer */
#define OMPI_SIZEOF_FORTRAN_INT 4

/* Size of fortran logical */
#define OMPI_SIZEOF_FORTRAN_LOGICAL 4

/* Size of fortran real */
#define OMPI_SIZEOF_FORTRAN_REAL 4

/* Do threads have different pids (pthreads on linux) */
/* #undef OMPI_THREADS_HAVE_DIFFERENT_PIDS */

/* Whether to use <stdbool.h> or not */
#define OMPI_USE_STDBOOL_H 0

/* Whether we want MPI cxx support or not */
#define OMPI_WANT_CXX_BINDINGS 1

/* Whether we want the MPI f77 bindings or not */
#define OMPI_WANT_F77_BINDINGS 0

/* Whether to include support for libltdl or not */
#define OMPI_WANT_LIBLTDL 

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* The size of a `char', as computed by sizeof. */
#define SIZEOF_CHAR 1

/* The size of a `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of a `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of a `long double', as computed by sizeof. */
#define SIZEOF_LONG_DOUBLE 8

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of a `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* Additional CFLAGS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_CFLAGS ""

/* Additional CXXFLAGS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_CXXFLAGS ""

/* Additional FCFLAGS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_FCFLAGS ""

/* Additional FFLAGS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_FFLAGS ""

/* Additional LDFLAGS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_LDFLAGS ""

/* Additional LIBS to pass through the wrapper compilers */
#define WRAPPER_EXTRA_LIBS " "

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
/* #undef YYTEXT_POINTER */

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline __inline
#endif

/* C type corresponding to Fortran DOUBLE PRECISION */
#define ompi_fortran_dblprec_t double

/* C type corresponding to Fortran LOGICAL */
#define ompi_fortran_integer_t int

/* C type corresponding to Fortran LOGICAL */
#define ompi_fortran_logical_t int

/* C type corresponding to Fortran REAL */
#define ompi_fortran_real_t float

/* Define to `int' if <sys/types.h> does not define. */
#define pid_t int
#define SIZEOF_PID_T  4

/* Define to equivalent of C99 restrict keyword, or to nothing if this is not
   supported. Do not define if restrict is supported directly. */
#define restrict 

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to unsigned int if you dont have it */
#ifndef WIN32
#define socklen_t unsigned int
#endif

/* Define to `unsigned short' if <sys/types.h> does not define. */
#define u_int16_t unsigned short

/* Define to `unsigned int' if <sys/types.h> does not define. */
#define u_int32_t unsigned int

/* Define to `unsigned long long' if <sys/types.h> does not define. */
#define u_int64_t unsigned long long

/* Define to `unsigned char' if <sys/types.h> does not define. */
#define u_int8_t unsigned char

/* defining OMPI_NEED_C_BOOL to 1 since this is used as a check in configure
   to do the right magic for windows. please FIXME */
#define OMPI_NEED_C_BOOL 1

/* the maximum size to which the fortran to C translation table can grow
   this is the minimum of _I32_MAX and max fortran integer value. Defining
   this to be _I32_MAX for now FIXME */
#define OMPI_FORTRAN_HANDLE_MAX _I32_MAX

#define OMPI_ASSEMBLY_ARCH OMPI_WINDOWS

#define OMPI_WANT_MPI2_ONE_SIDED 1

#define MCA_pml_DIRECT_CALL 0

#define SIZE_MAX ((size_t) 0)
#define UINT8_MAX 255

/* STDOUT and STDERR defines */
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define PATH_MAX MAXPATHLEN

#include "ompi_config_bottom.h"
#endif /* OMPI_CONFIG_H */

