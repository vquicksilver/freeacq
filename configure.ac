#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([freeacq], [0.1], [victor.quicksilver@gmail.com])
AC_CONFIG_SRCDIR([src/facqoperation.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE([dist-bzip2])
AC_CANONICAL_HOST

dnl #####################################################################
dnl # User arguments
dnl #####################################################################

AC_ARG_ENABLE(
	[debug],
	[AC_HELP_STRING([--enable-debug],
	[compile with debug support @<:@default=no@:>@])],
	[enable_debug="$enableval"],
	[enable_debug="no"]
)

AC_ARG_ENABLE(
	[profiling],
	[AC_HELP_STRING([--enable-profiling],
	[compile with profiling support @<:@default=no@:>@])],
	[enable_profiling="$enableval"],
	[enable_profiling="no"]
)

AC_ARG_ENABLE(
	[gtk],
	[AC_HELP_STRING([--enable-gtk],
	[compile with graphical modules @<:@default=yes@:>@])],
	[gtk="$enableval"],
	[gtk="yes"]
)

AC_ARG_ENABLE(
	[comedi],
	[AC_HELP_STRING([--enable-comedi],
	[compile with comedi data adquisition support @<:@default=no@:>@])],
	[comedi="$enableval"], 
	[comedi="no"]
)

AC_ARG_ENABLE(
	[nidaqmx],
	[AC_HELP_STRING([--enable-nidaqmx],
	[compile with nidaqmx data adquisition support @<:@default=no@:>@])],
	[nidaqmx="$enableval"], 
	[nidaqmx="no"]
)

AC_ARG_ENABLE(
	[nidaqmxbase],
	[AC_HELP_STRING([--enable-nidaqmxbase],
	[compile with nidaqmxbase data adquisition support @<:@default=no@:>@])],
	[nidaqmxbase="$enableval"],
	[nidaqmxbase="no"]
)

AC_ARG_ENABLE(
	[fftw3],
	[AC_HELP_STRING([--enable-fftw3],
	[compile with fftw3 support @<:@default=no@:>@])],
	[fftw3="$enableval"], 
	[fftw3="no"]
)

AC_DEFINE_UNQUOTED(TARGET_ALIAS, "${host}", [A string representing our host])
case "$host" in
	*-*-linux*)
		AC_DEFINE([TARGET_UNIX], [1], [Are we running on Unix?])
		;;
	*-*-darwin*)
		AC_DEFINE([TARGET_UNIX], [1], [Are we running on Unix?])
		;;
	*-mingw*)
		AC_DEFINE([TARGET_WIN32], [1], [Are we running on Windows?])
		LDFLAGS="${LDFLAGS} -Wl,-enable-stdcall-fixup"
		WIN32=yes
		;;
	*)
		AC_MSG_ERROR([Target not supported])
		;;
esac

dnl #####################################################################
dnl # i18n support
dnl #####################################################################
dnl # The AM_GNU_GETTEXT macro determines whether GNU gettext is 
dnl # available and should be used. If so, it sets the USE_NLS variable 
dnl # to ‘yes’; it defines ENABLE_NLS to 1 in the autoconf generated 
dnl # configuration file (usually called config.h); it sets the variables 
dnl # LIBINTL and LTLIBINTL to the linker options for use in a Makefile 
dnl # (LIBINTL for use without libtool, LTLIBINTL for use with libtool); 
dnl # it adds an ‘-I’ option to CPPFLAGS if necessary. 
dnl # 
dnl # In the negative case, it sets USE_NLS to ‘no’; it sets LIBINTL and 
dnl # LTLIBINTL to empty and doesn't change CPPFLAGS.
dnl #####################################################################
AM_GNU_GETTEXT([external])

dnl ##################################################################
dnl # Change flags if debug is enabled
dnl ##################################################################
if test "x$enable_debug" = "xyes" ; then
	CFLAGS="${CFLAGS} -ggdb -g -Wall"
	AC_DEFINE(ENABLE_DEBUG, 1, [Debug enabled])
fi

dnl ##################################################################
dnl # Change flags if profiling is enabled
dnl ##################################################################
if test "x$enable_profiling" = "xyes" ; then
	CFLAGS="${CFLAGS} -pg"
	AC_DEFINE(ENABLE_PROFILING, 1, [Profiling enabled])
fi

dnl ###################################################################
dnl # Only one from nidaqmx and nidaqmxbase can be enabled
dnl ###################################################################
if test "x$nidaqmx" = "xyes" ; then
	if test "x$nidaqmxbase" = "xyes"; then
		AC_MSG_ERROR([
Sorry but only one NIDAQ library can be enabled, you must enable NIDAQmx
or NIDAQmxBase.
		])
	fi
fi

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG
if test "${WIN32}" = "yes"; then
AC_CHECK_TOOL(
	[WINDRES], 
	[windres],
	[no]
)
if test "${WINDRES}" = "no"; then
AC_MSG_ERROR([windres not found])
fi
fi
# check for gtk-doc
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14], [--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

# Checks for libraries.
dnl #######################################################################
dnl # GLIB Check                                                          #
dnl #######################################################################
PKG_CHECK_MODULES(
	[GLIB],
	[glib-2.0 >= 2.22.0 gobject-2.0 gthread-2.0 gio-2.0], 
	, 
	[	AC_MSG_RESULT(no) 
		AC_MSG_ERROR([
You must have GLib 2.22.0 or newer development headers installed to build.

If you have these installed already you may need to install pkg-config so
I can find them.
		])
	]
)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl #######################################################################
dnl # GTK Check, plus gtkdatabox					  #
dnl #######################################################################
dnl # Future gtk+-3 check
dnl #if test "x$gtk" = "xyes" ; then
dnl #PKG_CHECK_MODULES(
dnl #	[GTK],
dnl #	[gtk+-3.0],
dnl #	[
dnl #		gtk3="yes"
dnl #		CFLAGS="${CFLAGS} -DGTK_DISABLE_SINGLE_INCLUDES"
dnl #		CFLAGS="${CFLAGS} -DGTK_DISABLE_DEPRECATED"
dnl #		CFLAGS="${CFLAGS} -DGSEAL_ENABLE"
dnl #	],
dnl #	[gtk3="no"]
dnl #)
dnl #if test "x$gtk3" = "xno" ; then
PKG_CHECK_MODULES(
	[GTK],
	[gtk+-2.0 >= 2.18], 
	[gtk2="yes"],	
	[	
			gtk2="no"
			AC_MSG_RESULT(no)
			AC_MSG_ERROR([
You must have GTK+ 2.18.0 and gtkdatabox-0.9.1.1 or 
newer development headers installed to build.
You can disable the graphical modules with --disable-gtk.
			])
	]
)
dnl #fi
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(
	[GTKDATABOX], 
	[gtkdatabox >= 0.9.1.1], 
	,
	[
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([
You must have GTKDATABOX installed.
You can disable the graphical modules with --disable-gtk.
		])
	]
)
AC_SUBST(GTKDATABOX_CFLAGS)
AC_SUBST(GTKDATABOX_LIBS)

AC_DEFINE(USE_GTK, 1, [Gtk gui enabled])	

dnl #fi

dnl #####################################################################
dnl # COMEDI								#
dnl #####################################################################
if test "x$comedi" = "xyes" ; then
AC_CHECK_HEADER(
	[comedilib.h],
	[AC_DEFINE([HAVE_COMEDILIB_H], 
	[1],
        [Define to 1 if you have <comedilib.h>.])],
	[
		AC_MSG_ERROR([	
sorry, but I need comedilib.h to build.
If you do not need it use --disable-comedi.
		])
	]
)

PKG_CHECK_MODULES(
	[COMEDI], 
	[comedilib >= 0.8], 
	, 
	[
		AC_MSG_RESULT(no)
		comedi="no"
		AC_MSG_ERROR([		
You must have comedilib version 0.8 or greater installed to build.

If you do not need it use --disable-comedi.
		])
	]
)

if test "x$comedi" = "xyes" ; then
	AC_DEFINE(USE_COMEDI, 1, [Comedi enabled])	
	AC_SUBST(COMEDI_CFLAGS)
	AC_SUBST(COMEDI_LIBS)
fi

fi

dnl ###################################################################
dnl # NIDAQmx
dnl ###################################################################
if test "x$nidaqmx" = "xyes" ; then
AC_CHECK_HEADERS(
	[NIDAQmx.h],
	[AC_DEFINE([HAVE_NIDAQMX_H],
	[1],
        [Define to 1 if you have <NIDAQmx.h>.])],
        [
		AC_MSG_ERROR([
		
sorry, but I need NIDAQmx.h to build. To let autoconf find it you
can set CPPFLAGS to point to it. On UNIX platforms it can be found
installed on /usr/local/include. On Windows platforms it depends
on your installation path.

If you do not need it use --disable-nidaqmx.
		])
	],
	[[
#ifndef __int64
#define __int64 long long
#endif
#include <NIDAQmx.h>
	]]
)
dnl ##########################################################################
dnl # Instead of searching for nidaqmx, we have to search for the nicaiu.dll #
dnl # else in won't work, we have to take a deeper look at why this happens  #
dnl # but anyway this workaround seems to be valid.                          #
dnl # According to the openvpn configure.ac this can be caused by stdcall    #
dnl ##########################################################################

NIDAQMX_LIBS=""
AC_CHECK_LIB(
	[nidaqmx],
	[DAQmxGetExtendedErrorInfo], 
	[
		nidaqmx="yes"
		NIDAQMX_LIBS="-lnidaqmx"
	], 
	nidaqmx="no"
)

if test "x$nidaqmx" = "xno" ; then
AC_CHECK_LIB(
	[nicaiu],
	[DAQmxGetExtendedErrorInfo],
	[
		nidaqmx="yes"
		NIDAQMX_LIBS="-lnicaiu"
	],
	nidaqmx="no"
)
fi

if test "x$nidaqmx" = "xyes" ; then
	AC_DEFINE(USE_NIDAQMX, 1, [NIDAQmx enabled])
	AC_SUBST([NIDAQMX_CFLAGS],[])
	AC_ARG_VAR([NIDAQMX_CFLAGS],[C compiler flags for NIDAQmx, overriding defaults])
	AC_SUBST([NIDAQMX_LIBS])
	AC_ARG_VAR([NIDAQMX_LIBS],[linker flags for NIDAQmx, overriding defaults])
else
	AC_MSG_ERROR([

You must have NIDAQmx libraries installed to build. On Windows systems nicaiu 
library is needed. On Linux you will need the nidaqmx library.
If installed try tweaking LDFLAGS for example LDFLAGS=-Lc:\WINDOWS\system32\.

If you do not need it use --disable-nidaqmx.
	])
fi

fi

dnl #######################################################################
dnl # NIDAQmxBase
dnl #######################################################################
if test "x$nidaqmxbase" = "xyes" ; then
AC_CHECK_HEADERS(
	[NIDAQmxBase.h],
	[AC_DEFINE([HAVE_NIDAQMXBASE_H],
	[1],
        [Define to 1 if you have <NIDAQmxBase.h>.])],
        [AC_MSG_ERROR([
Sorry, but I need NIDAQmxBase.h to build. The default installation
of NIDAQmx-Base-3.6.0 in UNIX places the NIDAQmxBase.h header file in 
/usr/local/natinst/nidaqmxbase/include. To let autoconf find it you
can set CPPFLAGS to point to it, or create a symlink with the following
command (as root): 
ln -sf /usr/local/natinst/nidaqmxbase/include/NIDAQmxBase.h /usr/local/include/NIDAQmxBase.h .

If you do not need it use --disable-nidaqmxbase.
	])],
	[
		[#include <NIDAQmxBase.h>]
	]
)
AC_CHECK_LIB(
	[nidaqmxbase],
	[DAQmxBaseCreateTask], 
	, 
	[
		AC_MSG_RESULT(no)
		nidaqmxbase="no"
		AC_MSG_ERROR([
You must have nidaqmxbase library to build.

If you do not need it use --disable-nidaqmxbase.
		])
	]
)

if test "x$nidaqmxbase" = "xyes" ; then
	AC_DEFINE(USE_NIDAQMXBASE, 1, [NIDAQmxBase enabled])
	AC_SUBST([NIDAQMXBASE_CFLAGS],[])
	AC_ARG_VAR([NIDAQMXBASE_CFLAGS],[C compiler flags for NIDAQmxBase, overriding defaults])
	AC_SUBST([NIDAQMXBASE_LIBS],["-lnidaqmxbase"])
	AC_ARG_VAR([NIDAQMXBASE_LIBS],[linker flags for NIDAQmxBase, overriding defaults])
fi

fi

dnl #######################################################################
dnl # FFTW3
dnl #######################################################################
if test "x$fftw3" = "xyes" ; then
AC_CHECK_HEADER(
	[fftw3.h],
	[
		AC_DEFINE(
				[HAVE_FFTW3_H], 
				[1],
				[Define to 1 if you have <fftw3.h>.]
		)
	],
	[
		AC_MSG_ERROR([
sorry, but I need fftw3.h to build.

If you do not want to install fftw3 use --disable-fftw3.
		])
	]
)

PKG_CHECK_MODULES(
	FFTW3,
	[fftw3], 
	, 
	[
		AC_MSG_RESULT(no)
		fftw3="no"
		AC_MSG_ERROR([	
You must have FFTW installed to build.

If you do not want fftw3 use --disable-fftw3.
		])
	]
)

if test "x$fftw3" = "xyes" ; then
	AC_DEFINE(USE_FFTW3, 1, [FFTW3 enabled])	
	AC_SUBST(FFTW3_CFLAGS)
	AC_SUBST(FFTW3_LIBS)
fi

fi

dnl ################################################################
dnl # If nidaqmx or nidaqmxbase is enabled set enable_nidaq to yes #
dnl # This will allow us to define USE_NIDAQ, this variable is set #
dnl # if any NIDAQ product is enabled.                             #
dnl ################################################################
enable_nidaq="no"
if test "x$nidaqmx" = "xyes" ; then
	enable_nidaq="yes"
fi

if test "x$nidaqmxbase" = "xyes" ; then
	enable_nidaq="yes"
fi

if test "x$enable_nidaq" = "xyes" ; then
	AC_DEFINE(USE_NIDAQ, 1 , [NIDAQ enabled])
fi

# Automake conditionals
AM_CONDITIONAL(ENABLE_NLS,test "x$USE_NLS" = "xyes")
AM_CONDITIONAL(ENABLE_DEBUG, test "x$enable_debug" = "xyes")
AM_CONDITIONAL(ENABLE_PROFILING, test "x$enable_profiling" = "xyes")
AM_CONDITIONAL(TARGET_WIN32, test "x$WIN32" = "xyes")
AM_CONDITIONAL(ENABLE_COMEDI, test "x$comedi" = "xyes")
AM_CONDITIONAL(ENABLE_NIDAQ, test "x$enable_nidaq" = "xyes")
AM_CONDITIONAL(ENABLE_NIDAQMX, test "x$nidaqmx" = "xyes")
AM_CONDITIONAL(ENABLE_NIDAQMXBASE, test "x$nidaqmxbase" = "xyes")
AM_CONDITIONAL(ENABLE_FFTW3, test "x$fftw3" = "xyes")
AM_CONDITIONAL(ENABLE_GTK, test "x$gtk" = "xyes")

# Checks for header files.
#AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
#AC_CHECK_FUNCS([bzero])

AC_CONFIG_FILES(
	[
		Makefile
		src/Makefile
		docs/Makefile
		docs/reference/Makefile
		docs/reference/freeacq/Makefile
		mingw/Makefile
		po/Makefile.in
	]
)
AC_OUTPUT

dnl ################################################################
dnl # Print configuration
dnl ################################################################
echo 
echo "Build Configuration:"
echo
if test "x$USE_NLS" = "xyes"; then
echo " * NLS support:                      yes"
else
echo " * NLS support:                      no"
fi
if test "x$enable_debug" = "xyes"; then
echo " * Debug build:                      yes"
else
echo " * Debug build:                      no"
fi
if test "x$enable_profiling" = "xyes"; then
echo " * Profiling enabled:                yes"
else
echo " * Profiling enabled:                no"
fi
if test "x$gtk" = "xyes"; then
echo " * Build GUI:                        yes"
else
echo " * Build GUI:                        no"
fi
dnl #if test "x$gtk3" = "xyes"; then
dnl #echo " * gtk+-3 support:                   yes"
dnl #fi
if test "x$gtk2" = "xyes"; then
echo " * gtk+-2 support:                   yes"
fi
if test "x$comedi" = "xyes"; then
echo " * Build COMEDI support:             yes"
else
echo " * Build COMEDI support:             no"
fi
if test "x$enable_nidaq" = "xyes"; then
echo " * Build with N.I. DAQ support:      yes"
else
echo " * Build with N.I. DAQ support:      no"
fi
if test "x$nidaqmx" = "xyes"; then
echo " * Build NIDAQmx support:            yes"
else
echo " * Build NIDAQmx support:            no"
fi
if test "x$nidaqmxbase" = "xyes"; then
echo " * Build NIDAQmxBase support:        yes"
else
echo " * Build NIDAQmxBase support:        no"
fi
if test "x$fftw3" = "xyes"; then
echo " * Build with FFTW3 support:         yes"
else
echo " * Build with FFTW3 support:         no"
fi
if test "x$enable_gtk_doc" = "xyes"; then
echo " * Build documentation:              yes"
else
echo " * Build documentation:              no"
fi
