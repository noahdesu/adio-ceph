# -*- shell-script -*-
#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2009-2011 Cisco Systems, Inc.  All rights reserved.
# $COPYRIGHT$
# 
# Additional copyrights may follow
# 
# $HEADER$
#
AC_DEFUN([MCA_orte_ess_slurmd_PRIORITY], [10])

# MCA_ess_slurmd_CONFIG([action-if-found], [action-if-not-found])
# -----------------------------------------------------------
AC_DEFUN([MCA_orte_ess_slurmd_CONFIG],[
    AC_CONFIG_FILES([orte/mca/ess/slurmd/Makefile])

    ORTE_CHECK_SLURM([ess_slurmd], [ess_slurmd_good=1], [ess_slurmd_good=0])
         
    # if check worked, set wrapper flags if so.  
    # Evaluate succeed / fail
    AS_IF([test "$ess_slurmd_good" = "1"],
          [ess_slurmd_WRAPPER_EXTRA_LDFLAGS="$ess_slurmd_LDFLAGS"
           ess_slurmd_WRAPPER_EXTRA_LIBS="$ess_slurmd_LIBS"
           $1],
          [$2])

    # set build flags to use in makefile
    AC_SUBST([ess_slurmd_CPPFLAGS])
    AC_SUBST([ess_slurmd_LDFLAGS])
    AC_SUBST([ess_slurmd_LIBS])

    # see if PMI support also requested
    ORTE_CHECK_PMI([ess_slurmd_pmi], [ess_slurmd_pmi_good=1], [ess_slurmd_pmi_good=0])

   # if check worked, set wrapper flags if so.  
    # Evaluate succeed / fail
    AS_IF([test "$ess_slurmd_pmi_good" = 1],
          [ess_slurmd_WRAPPER_EXTRA_LDFLAGS="$ess_slurmd_LDFLAGS $ess_slurmd_pmi_LDFLAGS"
           ess_slurmd_WRAPPER_EXTRA_LIBS="$ess_slurmd_LIBS $ess_slurmd_pmi_LIBS"
           ],
          [])

    # set build flags to use in makefile
    AC_SUBST([ess_slurmd_pmi_CPPFLAGS])
    AC_SUBST([ess_slurmd_pmi_LDFLAGS])
    AC_SUBST([ess_slurmd_pmi_LIBS])

])dnl
