##
## This file is part of ppm, a pretty printer for math
## Copyright (C) 2018 Nate Smith (nat2e.smith@gmail.com)
##
## ppm is free software: you can redistribute it and/or modify
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

AC_INIT([ppm], [1.0])
AM_MAINTAINER_MODE([disable])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
AC_PROG_LIBTOOL
AC_PROG_CXX

VERSION="1.0"
PACKAGE="ppm"

#AC_CHECK_LIB([taffy], [dcSystem_mark], [], [AC_MSG_ERROR([Taffy is required. Run build.sh first.])])

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

AC_PROG_INSTALL
AC_CANONICAL_HOST

AC_OUTPUT

cat<<EOF

EOF
