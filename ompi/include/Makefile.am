#
# Copyright (c) 2004-2009 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2006-2009 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2009      Oak Ridge National Labs.  All rights reserved.
# $COPYRIGHT$
# 
# Additional copyrights may follow
# 
# $HEADER$
#

# mpif-common.h is not generated, but mpif.h and mpif-config.h are.
# See big comments in these files for an explanation.

# note - headers and nodist_headers will go in ${includedir}/openmpi,
# not ${includedir}/
headers =
nodist_headers = \
	ompi_config.h

# Install these in $(includedir)
include_HEADERS =

# Always install these in $(pkgincludedir)
pkginclude_HEADERS =

if OMPI_WANT_F77_BINDINGS
include_HEADERS += \
	mpif-common.h \
	mpif-mpi-io.h
endif

# These files are always installed in $(includedir), but shouldn't be
# shipped since they are generated by configure from their .in
# counterparts (which AM automatically ships).
nodist_include_HEADERS = \
        mpi.h \
        mpi-ext.h \
        mpi_portable_platform.h

if OMPI_WANT_F77_BINDINGS
nodist_include_HEADERS += \
        mpif.h \
        mpif-config.h
endif

if WANT_INSTALL_HEADERS 
ompidir = $(includedir)/openmpi
nobase_dist_ompi_HEADERS = $(headers)
nobase_nodist_ompi_HEADERS = $(nodist_headers)
else
ompidir = $(includedir)
nobase_dist_noinst_HEADERS = $(headers)
nobase_nodist_noinst_HEADERS = $(nodist_headers)
endif

# Remove the auto-generated mpi-ext.h file
# Since there is no mpi-ext.h.in, autogen does not know to cleanup this file.
distclean-local:
	rm -f mpi-ext.h

include ompi/Makefile.am
