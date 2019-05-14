#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])

m4_define(ultracontrol_configure_ac)

#
# The following version number definitions
# Making releases:
#   ultracontrol_micro_version += 1;
#   ultracontrol_interface_age += 1;
#   ultracontrol_binary_age += 1;
# if any functions have been added, set interface_age to 0.
# if backwards compatibility has been broken,
# set binary_age _and_ interface_age to 0.
#
# in easier to understand terms:
#
# <mclasen> on the stable branch, interface age == micro
# <mclasen> on the unstable (ie master), interface age = 0

m4_define([ultracontrol_major_version], [2])
m4_define([ultracontrol_minor_version], [0])
m4_define([ultracontrol_micro_version], [1])
m4_define([ultracontrol_interface_age], [1])
m4_define([ultracontrol_binary_age],
          [m4_eval(100 * ultracontrol_minor_version + ultracontrol_micro_version)])
m4_define([ultracontrol_version],
          [ultracontrol_major_version.ultracontrol_minor_version.ultracontrol_micro_version])

# libtool version related macros
m4_define([ultracontrol_lt_release], [ultracontrol_major_version.ultracontrol_minor_version])
m4_define([ultracontrol_lt_current],
          [m4_eval(100 * ultracontrol_minor_version + ultracontrol_micro_version - ultracontrol_interface_age)])
m4_define([ultracontrol_lt_revision], [ultracontrol_interface_age])
m4_define([ultracontrol_lt_age], [m4_eval(ultracontrol_binary_age - ultracontrol_interface_age)])
m4_define([ultracontrol_lt_current_minus_age],
          [m4_eval(ultracontrol_lt_current - ultracontrol_lt_age)])

# if the minor version number is odd, then we want debugging.  Otherwise
# we only want minimal debugging support.
m4_define([ultracontrol_debug_default],
          [m4_if(m4_eval(ultracontrol_minor_version % 2), [1], [yes], [minimum])])dnl


AC_INIT(ultracontrol, [ultracontrol_version],
        [http://lar.com/enter_bug.cgi?product=ultracontrol])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4macros])

# Save this value here, since automake will set cflags later
cflags_set=${CFLAGS:+set}

AM_INIT_AUTOMAKE([1.11 -Wno-portability no-define no-dist-gzip dist-xz tar-ustar])
AM_MAINTAINER_MODE([enable])

# Support silent build rules. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
AM_SILENT_RULES([no])

# libtool versioning
LT_RELEASE=ultracontrol_lt_release
LT_CURRENT=ultracontrol_lt_current
LT_REVISION=ultracontrol_lt_revision
LT_AGE=ultracontrol_lt_age
LT_CURRENT_MINUS_AGE=ultracontrol_lt_current_minus_age
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_CURRENT_MINUS_AGE)

ULTRACONTROL_MAJOR_VERSION=ultracontrol_major_version
ULTRACONTROL_MINOR_VERSION=ultracontrol_minor_version
ULTRACONTROL_MICRO_VERSION=ultracontrol_micro_version
ULTRACONTROL_INTERFACE_AGE=ultracontrol_interface_age
ULTRACONTROL_BINARY_AGE=ultracontrol_binary_age
ULTRACONTROL_VERSION=ultracontrol_version

AC_SUBST(ULTRACONTROL_MAJOR_VERSION)
AC_SUBST(ULTRACONTROL_MINOR_VERSION)
AC_SUBST(ULTRACONTROL_MICRO_VERSION)
AC_SUBST(ULTRACONTROL_VERSION)
AC_SUBST(ULTRACONTROL_INTERFACE_AGE)
AC_SUBST(ULTRACONTROL_BINARY_AGE)

AC_DEFINE(ULTRACONTROL_MAJOR_VERSION, [ultracontrol_major_version],
	  [Define to the ULTRACONTROL major version])
AC_DEFINE(ULTRACONTROL_MINOR_VERSION, [ultracontrol__minor_version],
	  [Define to the ULTRACONTROL minor version])
AC_DEFINE(ULTRACONTROL_MICRO_VERSION, [ultracontrol__micro_version],
	  [Define to the ULTRACONTROL micro version])
AC_DEFINE(ULTRACONTROL_INTERFACE_AGE, [ultracontrol__interface_age],
	  [Define to the ULTRACONTROL interface age])
AC_DEFINE(ULTRACONTROL_BINARY_AGE, [ultracontrol__binary_age],
	  [Define to the ULTRACONTROL binary age])



dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_USE_SYSTEM_EXTENSIONS

AM_CONDITIONAL(HAVE_GCC, [test "$GCC" = "yes"])

AC_CANONICAL_HOST

dnl

dnl Check for a working C++ compiler, but do not bail out, if none is found.
AC_CHECK_TOOLS(CXX, [$CCC c++ g++ gcc CC cxx cc++ cl], [gcc])
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE(,[class a { int b; } c;], ,CXX=)
AM_CONDITIONAL(HAVE_CXX, [test "$CXX" != ""])
AC_LANG_RESTORE

AM_PROG_CC_C_O
AC_PROG_INSTALL

AC_PATH_PROG([DBUS_BINDING_TOOL], [dbus-binding-tool], [notfound])
AC_SYS_LARGEFILE

PKG_PROG_PKG_CONFIG(0.16)

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.40.0 gobject-2.0 gio-2.0 gio-unix-2.0 hiredis json-glib-1.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(MKT, [mktlib-2.0 mktbus-1.0 ultimate-1.0 ])
AC_SUBST(MKT_CFLAGS)
AC_SUBST(MKT_LIBS)


ALL_LINGUAS="`grep -v '^#' "$srcdir/po/LINGUAS" | tr '\n' ' '`"
AC_SUBST([CONFIG_STATUS_DEPENDENCIES],['$(top_srcdir)/po/LINGUAS'])

GETTEXT_PACKAGE=ultracontrol
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"],
  [Define the gettext package to be used])


dnl Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static win32-dll])


AM_GLIB_GNU_GETTEXT

dnl Compiler flags; macro originates from systemkt
dnl See https://bugzilla.gnome.org/show_bug.cgi?id=608953
AC_ARG_ENABLE(compile-warnings,[AS_HELP_STRING([--enable-compile-warnings],[Don't use builtin compiler warnings])],,enable_compile_warnings=no)

AC_ARG_ENABLE(test-mode,[AS_HELP_STRING([--test-mode],[Don't use builtin compiler warnings])],test_mode=yes,test_mode=no)
AS_IF([test "x$test_mode" == "xyes"], [
  dnl Test mode enabled
  TEST_MODE=1
  AC_SUBST(TEST_MODE)
  AC_DEFINE(TEST_MODE, [1],
	  [Enabled test mode flag])
],[test "x$test_mode" == "xno"], [
  dnl Test mode enabled
  TEST_MODE=0
  AC_SUBST(TEST_MODE)
  AC_DEFINE(TEST_MODE, [0],
	  [Disable test mode flag])
],[
  dnl Test mode enabled
  TEST_MODE=0
  AC_SUBST(TEST_MODE)
  AC_DEFINE(TEST_MODE, [0],
	  [Disable test mode flag])
])


EXTRA_WARN_CFLAGS="-Wall   \
                   -Wstrict-prototypes -Werror=missing-prototypes -Werror=implicit-function-declaration \
                   -Werror=pointer-arith -Werror=init-self -Werror=format-security \
                   -Werror=format=2 -Werror=missing-include-dirs"
#Zusatzliche Flags für höchste Prüfungsstufe.
#-Werror=declaration-after-statement


WARN_CFLAGS="-Wall"

if test "x$enable_compile_warnings" = xyes; then
	WARN_CFLAGS="$EXTRA_WARN_CFLAGS"
fi

AC_SUBST(WARN_CFLAGS)

AC_PREFIX_DEFAULT("/usr")

GLIB_GSETTINGS

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/axis-service/Makefile
	src/pumps-service/Makefile
	src/valves-service/Makefile
	src/stirrers-service/Makefile
	src/vessels_service/Makefile
	src/airflow-service/Makefile
	src/sequence-workings-service/Makefile
	src/measurement-service/Makefile
	po/Makefile.in
					])


AC_OUTPUT()
