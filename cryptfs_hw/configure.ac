AC_PREREQ(2.61)
AC_INIT([cryptfs_hw],1.0.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([LIBSELINUX], [libselinux])
AC_SUBST([LIBSELINUX_CFLAGS])
AC_SUBST([LIBSELINUX_LIBS])

PKG_CHECK_MODULES([LIBBASE], [libbase])

AC_ARG_WITH([emmc_use_ICE],
      AC_HELP_STRING([--with-emmc-use-ICE],
         [enable ICE harware encryption when the storage medium is emmc]))

AM_CONDITIONAL(USE_EMMC_ICE, [test "x${with_emmc_use_ICE}" = "xyes"])

AC_CONFIG_FILES([ \
        Makefile \
        libcryptfs_hw.pc
])
AC_OUTPUT
