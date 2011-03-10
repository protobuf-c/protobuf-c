AC_INIT([protobuf-c], [0.15])
AM_INIT_AUTOMAKE([1.9 foreign])
PACKAGE=protobuf-c
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PATH_PROG(PROTOC, protoc)
AC_CHECK_HEADERS(inttypes.h)
AC_CHECK_HEADERS(sys/poll.h)
AC_CHECK_HEADERS(sys/select.h)
AC_CHECK_HEADERS(alloca.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_HEADERS(io.h)
AC_CHECK_HEADERS(sys/uio.h)
AC_CHECK_HEADERS(unistd.h)

BUILD_PROTOC_C=1
AC_ARG_ENABLE(protoc, [  --disable-protoc    Suppress build of protoc_c],
              if test "x$enableval" = xno ; then
	        BUILD_PROTOC_C=0
              fi)
AM_CONDITIONAL(BUILD_PROTOC_C, test $BUILD_PROTOC_C = 1)

# --- Check for the protobuf library. ---
if test $BUILD_PROTOC_C = 1; then
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADER(google/protobuf/stubs/common.h,,
		  [AC_MSG_ERROR([
  ERROR: protobuf headers are required.

  You must either install protobuf from google,
  or if you have it installed in a custom location
  you must add '-Iincludedir' to CXXFLAGS
  and '-Llibdir' to LDFLAGS.

  If you did not specify a prefix when installing
  protobuf, try 
    './configure CXXFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib'
  In some 64-bit environments, try LDFLAGS=-L/usr/local/lib64.
  ])])
  pbc_savelibs="$LIBS"
  LIBS="$LIBS -lprotoc -lprotobuf -lpthread"
  AC_TRY_LINK([#include <google/protobuf/compiler/command_line_interface.h>],
	      [google::protobuf::compiler::CommandLineInterface cli;],
	      [],
	      [AC_MSG_ERROR([
  ERROR:
  protobuf test program failed to link:
  perhaps you need to add -Llibdir to your LDFLAGS.])])
  LIBS="$pbc_savelibs"
  AC_LANG_POP()

  dnl We need $PROTOC around for the test code generation.
  dnl This is merely needed for c++ packed-data comparison.
  if test "x$PROTOC" = x; then
    echo "ERROR: missing google's protoc program; adjust \$PATH (or use --disable-protoc)" 1>&2
    exit 1
  fi
fi

dnl  ------ define IS_LITTLE_ENDIAN ------
# We try to use, where possible the headers <endian.h>, <mach/endian.h>,
# and <machine/endian.h>, in that order.  They must define the macros
# __LITTLE_ENDIAN and __BYTE_ORDER.  We structure the test so that if they 
# do NOT define __LITTLE_ENDIAN or __BYTE_ORDER then we will disable
# the little-endian optimizations, so the resulting code should be correct,
# but not as fast, if there's a broken endian.h somewhere.
#
# If none of those headers exist, we fallback on a runtime test.
knows_endianness=0
AC_ARG_WITH([endianness],
  [AS_HELP_STRING([--with-endianness=[little|big]],
    [use the given endianness instead of testing headers (useful for mingw)])],
  [],[with_endianness=no])
AS_IF([test "$with_endianness" != "no"], [
  AS_IF([test "$with_endianness" == "little"], [
    is_little_endian=1
  ], [
    AS_IF([test "$with_endianness" == "big"], [
      is_little_endian=0
    ], [
      AC_MSG_FAILURE([--with-endianness argument must be 'little' or 'big'])
    ])
  ])
  knows_endianness=1
])

AS_IF([test $knows_endianness = 0], [
  AC_CHECK_HEADERS([endian.h], [has_endian_h=1; knows_endianness=1], [has_endian_h=0])
  if test $knows_endianness = 1 ; then
    AC_TRY_COMPILE([#include <endian.h>], [
      switch (1) { case __LITTLE_ENDIAN: break;
		   case __BYTE_ORDER: break; } ],
      [is_little_endian=0], [is_little_endian=1])
  else

    # ------------------ try <mach/endian.h>  ------------------
    AC_CHECK_HEADERS([mach/endian.h], [has_mach_endian_h=1; knows_endianness=1], [has_mach_endian_h=0])
    if test $knows_endianness = 1 ; then
      AC_TRY_COMPILE([#include <mach/endian.h>],[
        switch (1) { case __LITTLE_ENDIAN: break;
		     case __BYTE_ORDER: break; } ],
        [is_little_endian=0], [is_little_endian=1])
    fi
    # ------------------ try <machine/endian.h>  ------------------
    if test $knows_endianness = 0; then
      AC_CHECK_HEADERS([machine/endian.h], [has_machine_endian_h=1; knows_endianness=1], [has_machine_endian_h=0])
      if test $knows_endianness = 1 ; then
        AC_TRY_COMPILE([#include <machine/endian.h>],[
	  switch (1) { case __LITTLE_ENDIAN: break;
		       case __BYTE_ORDER: break; } ],
	  [is_little_endian=0], [is_little_endian=1])
      fi
    fi
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
])

if test $is_little_endian = 1; then
  echo "Your system IS little-endian" 1>&2
else
  echo "Your system IS NOT little-endian" 1>&2
fi
AC_DEFINE_UNQUOTED(IS_LITTLE_ENDIAN, $is_little_endian)

AC_OUTPUT(Makefile src/Makefile src/test/Makefile pkgwriteinfo libprotobuf-c.pc)


