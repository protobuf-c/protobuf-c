AC_PREREQ(2.64)

AC_INIT([protobuf-c],
        [1.0.2],
        [https://github.com/protobuf-c/protobuf-c/issues],
        [protobuf-c],
        [https://github.com/protobuf-c/protobuf-c])
PACKAGE_DESCRIPTION="Protocol Buffers implementation in C"
AC_SUBST(PACKAGE_DESCRIPTION)

AC_CONFIG_SRCDIR([protobuf-c/protobuf-c.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules subdir-objects])
AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile protobuf-c/libprotobuf-c.pc])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN],
               [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN],
           [AC_CONFIG_FILES([Doxyfile])
            DOXYGEN_INPUT="${srcdir}/protobuf-c"
            AC_SUBST(DOXYGEN_INPUT)
           ])

PKG_PROG_PKG_CONFIG
if test -n "$PKG_CONFIG"; then
    # Horrible hack for systems where the pkg-config install directory is simply wrong!
    if $PKG_CONFIG --variable=pc_path pkg-config 2>/dev/null | grep -q /libdata/; then
        PKG_INSTALLDIR(['${prefix}/libdata/pkgconfig'])
    else
        PKG_INSTALLDIR
    fi
fi

AC_ARG_ENABLE([protoc],
  AS_HELP_STRING([--disable-protoc], [Disable building protoc_c (also disables tests)]))
if test "x$enable_protoc" != "xno"; then
  AC_LANG_PUSH([C++])
  PKG_CHECK_MODULES([protobuf], [protobuf >= 2.5.0])

  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$save_CPPFLAGS $protobuf_CFLAGS"
  AC_CHECK_HEADERS([google/protobuf/compiler/command_line_interface.h],
                   [],
                   [AC_MSG_ERROR([required protobuf header file not found])])
  CPPFLAGS="$save_CPPFLAGS"

  AC_ARG_VAR([PROTOC], [protobuf compiler command])
  AC_PATH_PROG([PROTOC], [protoc], [],
               [`$PKG_CONFIG --variable=exec_prefix protobuf`/bin:$PATH])
  if test -z "$PROTOC"; then
    AC_MSG_ERROR([Please install the protobuf compiler from https://code.google.com/p/protobuf/.])
  fi
  PROTOBUF_VERSION="$($PROTOC --version)"
else
  PROTOBUF_VERSION="not required, not building compiler"
fi
AM_CONDITIONAL([BUILD_COMPILER], [test "x$enable_protoc" != "xno"])

gl_LD_VERSION_SCRIPT

gl_VALGRIND_TESTS

MY_CODE_COVERAGE

AC_C_BIGENDIAN

AC_OUTPUT
AC_MSG_RESULT([
    $PACKAGE $VERSION

        CC:                     ${CC}
        CFLAGS:                 ${CFLAGS}
        CXX:                    ${CXX}
        CXXFLAGS:               ${CXXFLAGS}
        LDFLAGS:                ${LDFLAGS}
        LIBS:                   ${LIBS}

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}
        pkgconfigdir:           ${pkgconfigdir}

        bigendian:              ${ac_cv_c_bigendian}
        protobuf version:       ${PROTOBUF_VERSION}
])
