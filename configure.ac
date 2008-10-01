AC_INIT(src/google/protobuf-c/protobuf-c.h)
PROTOBUF_C_VERSION=0.5

AM_INIT_AUTOMAKE(protobuf-c, $PROTOBUF_C_VERSION)
PACKAGE=protobuf-c
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PATH_PROG(PROTOC, protoc)
AC_CHECK_HEADERS(inttypes.h)

dnl  ------ define IS_LITTLE_ENDIAN ------
knows_endianness=0
AC_CHECK_HEADERS([endian.h], [has_endian_h=1; knows_endianness=1], [has_endian_h=0])
if test $knows_endianness = 1 ; then
  AC_TRY_COMPILE([#include <endian.h>], [
   switch (1) { case __LITTLE_ENDIAN: break;
		case __BYTE_ORDER: break; } ],
    [is_little_endian=0], [is_little_endian=1])
else
  AC_CHECK_HEADERS([mach/endian.h], [has_mach_endian_h=1; knows_endianness=1], [has_mach_endian_h=0])
  AC_TRY_COMPILE([#include <mach/endian.h>],[
    switch (1) { case __LITTLE_ENDIAN: break;
		 case __BYTE_ORDER: break; }
			    ],
    [is_little_endian=0], [is_little_endian=1])
  if test $knows_endianness = 0; then
    AC_MSG_CHECKING([for little-endianness via runtime check])
    AC_RUN_IFELSE([#include <inttypes.h>
    int main() {
      uint32_t v = 0x01020304;
      return memcmp (&v, "\4\3\2\1", 4) == 0 ? 0 : 1;
    }
    ], [is_little_endian=1; result=yes], [is_little_endian=0; result=no])
    AC_MSG_RESULT($result)
  fi
fi

if test $is_little_endian = 1; then
  echo "Your system IS little-endian" 1>&2
else
  echo "Your system IS NOT little-endian" 1>&2
fi
AC_DEFINE_UNQUOTED(IS_LITTLE_ENDIAN, $is_little_endian)

AC_OUTPUT( Makefile src/Makefile src/test/Makefile pkgwriteinfo )


