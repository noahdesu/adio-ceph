dnl -*- shell-script -*-
dnl
dnl $HEADER$
dnl

#
# LAM/MPI-specific tests
#

sinclude(config/c_weak_symbols.m4)

sinclude(config/cxx_find_template_parameters.m4)
sinclude(config/cxx_find_template_repository.m4)
sinclude(config/cxx_have_exceptions.m4)
sinclude(config/cxx_find_exception_flags.m4)

sinclude(config/f77_find_ext_symbol_convention.m4)

sinclude(config/lam_case_sensitive_fs_setup.m4)
sinclude(config/lam_check_optflags.m4)
sinclude(config/lam_config_subdir.m4)
sinclude(config/lam_config_subdir_args.m4)
sinclude(config/lam_configure_options.m4)
sinclude(config/lam_functions.m4)
sinclude(config/lam_get_version.m4)
sinclude(config/lam_mca.m4)
sinclude(config/lam_setup_cc.m4)
sinclude(config/lam_setup_cxx.m4)
sinclude(config/lam_setup_f77.m4)
sinclude(config/lam_setup_f90.m4)

sinclude(config/lam_check_pthread_pids.m4)
sinclude(config/lam_config_pthreads.m4)
sinclude(config/lam_config_solaris_threads.m4)
sinclude(config/lam_config_threads.m4)
