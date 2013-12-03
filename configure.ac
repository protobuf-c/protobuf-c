AC_PREREQ(2.60)
AC_INIT([protobuf-c], [0.16])
AC_CONFIG_SRCDIR([protobuf-c/protobuf-c.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules subdir-objects])
AC_PROG_CC_STDC
AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile protobuf-c/libprotobuf-c.pc protobuf-c-rpc/libprotobuf-c-rpc.pc])

AC_CHECK_HEADERS(alloca.h)
AC_CHECK_HEADERS(inttypes.h)
AC_CHECK_HEADERS(io.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(sys/poll.h)
AC_CHECK_HEADERS(sys/select.h)
AC_CHECK_HEADERS(sys/uio.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(winsock.h)

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

PKG_CHECK_MODULES([protobuf], [protobuf])
AC_LANG_PUSH([C++])
AC_PATH_PROG(PROTOC, protoc)

AC_ARG_ENABLE(
    [rpc],
    AS_HELP_STRING([--enable-rpc], [Enable building the protobuf-c rpc library])
)
AM_CONDITIONAL([BUILD_RPC], [test "x$enable_rpc" = "xyes"])

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
        protobuf version:       $($PROTOC --version)
])
