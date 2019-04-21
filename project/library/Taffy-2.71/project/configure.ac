##
## This file is part of Taffy, a mathematical programming language.
## Copyright (C) 2016-2017 Arithmagic, LLC
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

AC_INIT([taffy], [2.53])
AM_MAINTAINER_MODE([disable])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
AC_PROG_LIBTOOL
AC_PROG_OBJC

VERSION="2.53"
PACKAGE="taffy"

# library checks #
AC_CHECK_LIB(pthread,
             pthread_create,
             ,
             [AC_MSG_ERROR([pthreads is not found])])

AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([m], [ceil])
AC_CHECK_LIB([m], [floor])
AC_CHECK_LIB([dl], [dlopen])

CFLAGS_OUTPUT="you may need to set your CFLAGS and LDFLAGS environment variables appropriately. Example: CFLAGS=-I/opt/local/include LDFLAGS=-L/opt/local/lib"

# yacc or bison is required
AC_CHECK_PROG([YACC],
              [bison],
              [bison],
              [AC_MSG_ERROR([YACC/bison not found.])])

# checks for header files #
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h termios.h unistd.h stddef.h libintl.h])
AC_CHECK_HEADERS(pthread.h)

# checks for typedefs, structures, and compiler characteristics #
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_HEADER_STDBOOL

AC_C_VOLATILE

# checks for library functions #
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([log ceil gethostbyname pow setenv socket sqrt strdup])
AC_PROG_CC

# for intermixing with objective-c
AM_PROG_CC_C_O

AC_PROG_INSTALL
AC_CANONICAL_HOST

case $host_os in
  darwin*)
    MY_APPLE=true
    ;;
  linux*)
    MY_LINUX=true
    ;;
  cygwin*)
    MY_CYGWIN=true
    ;;
  *)
    ;;
esac

# let Makefile.am know whether we're apple or linux
AM_CONDITIONAL([APPLE], [test x$MY_APPLE = "xtrue"])
AM_CONDITIONAL([LINUX], [test x$MY_LINUX = "xtrue"])
AM_CONDITIONAL([CYGWIN], [test x$MY_CYGWIN = "xtrue"])

#AC_SUBST([libdir], [${EXEC_PREFIX}/lib/taffy])
#     Binary installation directory: '${EXEC_PREFIX}'
#     Library directory: ${pkglibdir}

#
# the --with-object-tip option
#
AC_ARG_WITH([garbage-collector-object-tip],
            [AS_HELP_STRING([--with-garbage-collector-object-tip],
               [set the garbage collector's object ip])],
            [AC_DEFINE_UNQUOTED([GARBAGE_COLLECTOR_OBJECT_TIP], [$withval], [custom garbage collector object tip])],
            [AC_DEFINE_UNQUOTED([GARBAGE_COLLECTOR_OBJECT_TIP], [6000000], [default garbage collector object tip])])

#
# the --enable-pie argument
# Pie is compiled by default
#
AC_ARG_ENABLE([pie],
              [  --enable-pie               build Pie [[default=yes]]
  --disable-pie              don't build Pie],
              [case "${enableval}" in
               yes) pie=true ;;
               no) pie=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-pie]) ;;
               esac],
              [pie=true])

# add variable for Makefile.am
AM_CONDITIONAL([COMPILE_PIE], [test "x$pie" = "xtrue"])

haveReadlineHeaders=false

AC_CHECK_HEADERS(readline/readline.h editline/readline.h,
                 haveReadlineHeaders=true,
                 [])

# add local variable
AS_IF([test "x$pie" = "xtrue"],
      AS_IF([test "x$haveReadlineHeaders" = "xtrue"],
            [PIE_YES_NO=y],
            [AC_MSG_ERROR([please install readline/editline headers or disable pie via --disable-pie])]),
      [PIE_YES_NO=n])

AS_IF([test "x$pie" = "xtrue"],
      [AC_SEARCH_LIBS([readline],
                      [readline editline],
                      [PIE_YES_NO=y],
                      [AC_MSG_ERROR([please install readline/editline libs or disable pie via --disable-pie])])],
      [PIE_YES_NO=n])

AM_COND_IF([COMPILE_PIE],
           [AC_DEFINE([COMPILE_PIE], 1, [compile Pie])],
           [AC_DEFINE([COMPILE_PIE], 0, [do not compile Pie])])
#
# /--enable-external-io
#

#
# the --enable-external-io argument
# External IO is compiled by default
#
AC_ARG_ENABLE([external-io],
              [  --enable-external-io               build External IO (File and Socket) [[default=yes]]
  --disable-external-io              don't build External IO],
              [case "${enableval}" in
               yes) externalIO=true ;;
               no) externalIO=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-external-io]) ;;
               esac],
              [externalIO=true])

# add variable for Makefile.am
AM_CONDITIONAL([COMPILE_EXTERNAL_IO], [test "x$externalIO" = "xtrue"])

# add local variable
AS_IF([test "x$externalIO" = "xtrue"],
      [COMPILE_EXTERNAL_IO_YES_NO=y],
      [COMPILE_EXTERNAL_IO_YES_NO=n])

AM_COND_IF([COMPILE_EXTERNAL_IO],
           [AC_DEFINE([COMPILE_EXTERNAL_IO], 1, [compile External IO])],
           [])
#
# /--enable-external-io
#

#
# the --enable-debug argument
# debug is not turned on by default
#
AC_ARG_ENABLE([debug],
              [  --enable-debug             enable debug logging [[default=no]]
  --disable-debug            disable debug logging],
              [case "${enableval}" in
               yes) debug=true ;;
               no) debug=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
              [debug=false])

# add variable for Makefile.am
AM_CONDITIONAL([DEBUG_BUILD], [test "x$debug" = "xtrue"])

# add local variable
AS_IF([test "x$debug" = "xtrue"],
      [DEBUG_BUILD_YES_NO=y],
      [DEBUG_BUILD_YES_NO=n])

# add #define for config.h
AM_COND_IF([DEBUG_BUILD],
           [AC_DEFINE([ENABLE_DEBUG], 1, [enable debug])],
           [])

#           [AC_DEFINE([ENABLE_DEBUG], 0, [disable debug])])
#
# /--enable-debug
#

#
# the --enable-execute-on-system argument
# tests are not built by default
#
AC_ARG_ENABLE([execute-on-system],
              [  --enable-execute-on-system  enable system execution via kernel.executeOnSystem: [[default=no]
  --disable-execute-on-system disable system execution]],
              [case "${enableval}" in
               yes) executeOnSystem=true ;;
               no) executeOnSystem=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-execute-on-system]) ;;
               esac],
              [executeOnSystem=false])

# add variable for Makefile.am
AM_CONDITIONAL([ENABLE_EXECUTE_ON_SYSTEM], [test "x$executeOnSystem" = "xtrue"])

# add local variable
AS_IF([test "x$executeOnSystem" = "xtrue"],
      [ENABLE_EXECUTE_ON_SYSTEM_YES_NO=y],
      [ENABLE_EXECUTE_ON_SYSTEM_YES_NO=n])

# add #define for config.h
AM_COND_IF([ENABLE_EXECUTE_ON_SYSTEM],
           [AC_DEFINE([ENABLE_EXECUTE_ON_SYSTEM], 1, [enable execute on system])],
           [])

#           [AC_DEFINE([ENABLE_EXECUTE_ON_SYSTEM], 0, [])])
#
# /--enable-execute-on-system
#

#
# the --enable-tests argument
# tests are not built by default
#
AC_ARG_ENABLE([tests],
              [  --enable-tests             build the Taffy C test suite [[default=no]]],
              [case "${enableval}" in
               yes) tests=true ;;
               no) tests=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-tests]) ;;
               esac],
              [tests=false])

# add variable for Makefile.am
AM_CONDITIONAL([COMPILE_TESTS], [test "x$tests" = "xtrue"])

# add local variable
AS_IF([test "x$tests" = "xtrue"],
      [COMPILE_TESTS_YES_NO=y],
      [COMPILE_TESTS_YES_NO=n])

# add #define for config.h
AM_COND_IF([COMPILE_TESTS],
           [AC_DEFINE([COMPILE_TESTS], 1, [build tests])],
           [AC_DEFINE([COMPILE_TESTS], 0, [do not build tests])])
#
# /--enable-tests
#

AC_OUTPUT

cat<<EOF

#######################
#                     #
# Build Configuration #
#                     #
#######################

[[$COMPILE_TESTS_YES_NO]] Compile C test suite
[[$DEBUG_BUILD_YES_NO]] Enable global debug logging
[[$COMPILE_EXTERNAL_IO_YES_NO]] Compile External IO (File, Socket)

########################
#                      #
# Target Configuration #
#                      #
########################

[[$PIE_YES_NO]] Compile Pie, the interactive Taffy interpreter
[[y]] Compile Taffy

EOF
