# Process this file with autoconf to produce a configure script.

AC_INIT([moodriver], [0.09])
AC_PREREQ([2.59])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.8])

AC_CONFIG_HEADERS([config.h])

# Checks for various programs
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname memset socket])

# Check for C compiler
AC_LANG([C])
AC_LANG([C++])
AC_ISC_POSIX

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname socket])

# Next four lines is a hack to prevent libtool checking for C++/F77
m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])

# Libtool
AC_DISABLE_STATIC
AC_ENABLE_SHARED
AC_PROG_LIBTOOL

# pkg-config
PKG_PROG_PKG_CONFIG
AC_ARG_WITH([pkgconfig-dir],
    AS_HELP_STRING([--with-pkgconfig-dir@<:@=PATH@:>@],
        [pkgconfig file destination dir]),
    [pkgconfigdir="$withval"],
    [pkgconfigdir='$(libdir)/pkgconfig'])
AC_SUBST([pkgconfigdir])

# GLIB
PKG_CHECK_MODULES([MOO], [glib-2.0 >= 2.8.0 gobject-2.0 gthread-2.0 sigc++-2.0])

# CFLAGS
if test "$ac_cv_c_compiler_gnu" = "yes"; then
       CFLAGS="$CFLAGS -Wall -pedantic -std=c99"
       CPPFLAGS="$CPPFLAGS -Wall -pedantic"
fi

# Add -Werr when building from SVN
if test -d .svn ; then
       CFLAGS="$CFLAGS -Werror"
       CPPFLAGS="$CPPFLAGS -Werror"
fi

AC_CONFIG_FILES([
    Makefile
    data/Makefile
    include/Makefile
    include/moo/Makefile
    src/Makefile
    example/Makefile
    moodriver.pc
])

AC_OUTPUT
