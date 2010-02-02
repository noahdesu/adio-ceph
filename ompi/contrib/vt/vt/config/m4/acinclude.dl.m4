AC_DEFUN([ACVT_DL],
[
	dl_error="no"
	have_dl="no"

	DLDIR=
	DLINCDIR=
	DLLIBDIR=
	DLLIB=

	AC_ARG_WITH(dl-dir,
		AC_HELP_STRING([--with-dl-dir=DLDIR], [give the path for libdl, default: /usr]),
	[DLDIR="$withval/"])

	AC_ARG_WITH(dl-inc-dir,
		AC_HELP_STRING([--with-dl-inc-dir=DLINCDIR],
		[give the path for libdl-include files, default: DLDIR/include]),
	[DLINCDIR="-I$withval/"],
	[AS_IF([test x"$DLDIR" != x], [DLINCDIR="-I$DLDIR"include/])])

	AC_ARG_WITH(dl-lib-dir,
		AC_HELP_STRING([--with-dl-lib-dir=DLLIBDIR],
		[give the path for libdl-libraries, default: DLDIR/lib]),
	[DLLIBDIR="-L$withval/"],
	[AS_IF([test x"$DLDIR" != x], [DLLIBDIR="-L$DLDIR"lib/])])

	AC_ARG_WITH(dl-lib,
		AC_HELP_STRING([--with-dl-lib=DLLIB], [use given libdl lib, default: -ldl]),
	[DLLIB="$withval"])

	sav_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$CPPFLAGS $DLINCDIR"
	AC_CHECK_HEADER([dlfcn.h], [],
	[
		AC_MSG_NOTICE([error: no dlfcn.h found; check path for libdl package first...])
		dl_error="yes"
	])
	CPPFLAGS=$sav_CPPFLAGS

	AS_IF([test x"$DLLIB" = x -a x"$dl_error" = "xno"],
	[
		sav_LIBS=$LIBS
		LIBS="$LIBS $DLLIBDIR -ldl"
		AC_MSG_CHECKING([whether linking with -ldl works])
		AC_TRY_LINK([],[],
		[AC_MSG_RESULT([yes]); DLLIB=-ldl],[AC_MSG_RESULT([no])])
		LIBS=$sav_LIBS
	])

	AS_IF([test x"$DLLIB" = x -a x"$dl_error" = "xno"],
	[
		AC_MSG_NOTICE([error: no libdl found; check path for libdl package first...])
		dl_error="yes"
	])

	AS_IF([test x"$dl_error" = "xno"],
	[
		have_dl="yes"
		sav_CPPFLAGS=$CPPFLAGS
                CPPFLAGS="$CPPFLAGS $DLINCDIR -D_GNU_SOURCE"
                AC_CHECK_DECLS([RTLD_NEXT], [], [], [#include <dlfcn.h>])
                CPPFLAGS=$sav_CPPFLAGS
	])

	AC_SUBST(DLINCDIR)
	AC_SUBST(DLLIBDIR)
	AC_SUBST(DLLIB)
])

