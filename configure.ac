  AC_PREREQ(2.61)
  AC_INIT([dtbtool],1.0.0)

  AM_INIT_AUTOMAKE([-Wall gnu foreign])
  AM_MAINTAINER_MODE

  AC_CONFIG_HEADER([config.h])
  AC_CONFIG_MACRO_DIR([m4])

  # Checks for programs.
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

  AS_CASE([$host],
          [arm*], [ARM=yes],
          [ARM=no]
  )

  AC_OUTPUT([ \
          Makefile \
          dtbtool/Makefile 
  ])
