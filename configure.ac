#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([picoquant], [0.01])
AC_CONFIG_SRCDIR([src/pq_g2.c])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([1.9 foreign])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [floor])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor])

# Checks required by automake
AC_PROG_CC

# Options
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
