# Copyright 2012 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

# Checks for existence of coverage tools:
#  * gcov
#  * lcov
#  * genhtml
#  * gcovr
#
# Sets ac_cv_check_gcov to yes if tooling is present
# and reports the executables to the variables LCOV, GCOVR and GENHTML.
AC_DEFUN([AC_TDD_GCOV],
[
  AC_ARG_ENABLE(gcov,
  AS_HELP_STRING([--enable-gcov],
		 [enable coverage testing with gcov]),
  [use_gcov=yes], [use_gcov=no])

  AM_CONDITIONAL(HAVE_GCOV, test "x$use_gcov" = "xyes")

  if test "x$use_gcov" = "xyes"; then
  # we need gcc:
  if test "$GCC" != "yes"; then
    AC_MSG_ERROR([GCC is required for --enable-gcov])
  fi

  # Check if ccache is being used
  AC_CHECK_PROG(SHTOOL, shtool, shtool)
  if test "$SHTOOL"; then
    AS_CASE([`$SHTOOL path $CC`],
                [*ccache*], [gcc_ccache=yes],
                [gcc_ccache=no])
  fi

  if test "$gcc_ccache" = "yes" && (test -z "$CCACHE_DISABLE" || test "$CCACHE_DISABLE" != "1"); then
    AC_MSG_ERROR([ccache must be disabled when --enable-gcov option is used. You can disable ccache by setting environment variable CCACHE_DISABLE=1.])
  fi

  AC_CHECK_PROG(LCOV, lcov, lcov)
  lcov_version=`$LCOV -v 2>/dev/null | $SED -e 's/^.* //'`
  AX_COMPARE_VERSION([$lcov_version], ge, [1.6], [],
                     [AC_MSG_ERROR([lcov version must be 1.6 or greater.])])

  AC_CHECK_PROG(GENHTML, genhtml, genhtml)

  if test -z "$GENHTML"; then
    AC_MSG_ERROR([Could not find genhtml from the lcov package])
  fi

  # Remove all optimization flags from CFLAGS
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9]*//g'`
  changequote([,])

  # Add the special gcc flags
  COVERAGE_CFLAGS="--coverage"
  COVERAGE_CXXFLAGS="--coverage"
  COVERAGE_LDFLAGS="-lgcov"

fi
]) # AC_TDD_GCOV
