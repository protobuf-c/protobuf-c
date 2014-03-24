AC_PREREQ(2.60)
AC_INIT([protobuf-c], [1.0.0-pre])
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

PKG_PROG_PKG_CONFIG
if test -n "$PKG_CONFIG"; then
    # Horrible hack for systems where the pkg-config install directory is simply wrong!
    if $PKG_CONFIG --variable=pc_path pkg-config 2>/dev/null | grep -q /libdata/; then
        PKG_INSTALLDIR(['${prefix}/libdata/pkgconfig'])
    else
        PKG_INSTALLDIR
    fi
else
    AC_MSG_ERROR([pkg-config is required!])
fi

DX_DOXYGEN_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_MAN_FEATURE(ON)
DX_INIT_DOXYGEN([protobuf-c], [Doxyfile], [doxygen-doc])
AS_IF([test "x$DX_DOXYGEN" != "x"], [has_doxygen=yes], [has_doxygen=no])
AM_CONDITIONAL([DOXYGEN_TARGET], [test "x$has_doxygen" = "xyes"])

AC_ARG_ENABLE([protoc],
  AS_HELP_STRING([--disable-protoc], [Disable building protoc_c (also disables tests)]))
if test "x$enable_protoc" != "xno"; then
  PKG_CHECK_MODULES([protobuf], [protobuf])
  AC_LANG_PUSH([C++])
  AC_PATH_PROG(PROTOC, protoc)
  if test -z "$PROTOC"; then
    AC_MSG_ERROR([Please install the protobuf compiler from https://code.google.com/p/protobuf/.])
  fi
  PROTOBUF_VERSION="$($PROTOC --version)"
else
  PROTOBUF_VERSION="not required, not building compiler"
fi
AM_CONDITIONAL([BUILD_COMPILER], [test "x$enable_protoc" != "xno"])

AC_TDD_GCOV
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)
gl_VALGRIND_TESTS

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
        doxygen docs:           ${has_doxygen}
])
