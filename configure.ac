AC_INIT([flang], [1.0], [llafuente@noboxout.com])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CANONICAL_HOST

AC_PROG_CC
AM_PROG_CC_C_O

# fix last automake error ~1.13.4
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

AC_PROG_LIBTOOL
LT_INIT

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
