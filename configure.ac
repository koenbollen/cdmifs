AC_INIT( [cdmifs], [0.0], [meneer@koenbollen.nl])
AM_INIT_AUTOMAKE( [-Wall -Werror foreign] ) 

AC_PROG_INSTALL
AC_PROG_CC 

AC_CONFIG_HEADERS([config.h]) 
AC_CONFIG_FILES([ 
				 Makefile 
				 src/Makefile 
				 ]) 

PKG_PROG_PKG_CONFIG(0.20) 
 
 
FUSE_REQUIRED_VERSION=2.6.0 
PKG_CHECK_MODULES(FUSE, fuse >= $FUSE_REQUIRED_VERSION) 
AC_SUBST(FUSE_CFLAGS) 
AC_SUBST(FUSE_LIBS) 

LIBCURL_CHECK_CONFIG( [yes], [7.19.0], [], AC_MSG_ERROR([libcurl not found]) )
 
AC_OUTPUT 
