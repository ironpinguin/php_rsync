dnl $Id$
dnl config.m4 for extension rsync

dnl Comments in this file start with the string 'dnl'.

PHP_ARG_WITH(rsync, for rsync support,
[  --with-rsync             Include rsync support])

if test "$PHP_RSYNC" != "no"; then
  dnl # --with-rsync -> check with-path
  SEARCH_PATH="/usr/local /usr" 
  SEARCH_FOR="include/librsync.h"
  if test -r $PHP_RSYNC/$SEARCH_FOR; then # path given as parameter
    RSYNC_DIR=$PHP_RSYNC
  else # search default path list
    AC_MSG_CHECKING([for librsync files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        RSYNC_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$RSYNC_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_CHECKING([for rsync library to use])
    AC_MSG_RESULT([bundled]) 

    if test ! -d librsync; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please install librsync or put source unter librsync directory here])
    fi
    dnl doing the bundled librsync config 
    cd librsync
    ./configure
    cd ..

    librsync_sources="librsync/prototab.c librsync/base64.c librsync/buf.c \
    librsync/checksum.c librsync/command.c librsync/delta.c librsync/emit.c \
    librsync/fileutil.c librsync/hex.c librsync/job.c librsync/mdfour.c \
    librsync/mksum.c librsync/msg.c librsync/netint.c \
    librsync/patch.c librsync/readsums.c \
    librsync/rollsum.c librsync/scoop.c librsync/search.c librsync/stats.c \
    librsync/stream.c librsync/sumset.c librsync/trace.c librsync/tube.c \
    librsync/util.c librsync/version.c librsync/whole.c"
    PHP_NEW_EXTENSION(rsync, $librsync_sources rsync.c, $ext_shared,,-I./librsync)
    
    PHP_ADD_BUILD_DIR($ext_builddir/librsync) 
dnl  PHP_INSTALL_HEADERS([ext/rsync], [php_rsync.h rsync/]) 
    AC_DEFINE(HAVE_BUNDLED_RSYNC, 1, [ ])
    AC_CHECK_FUNCS([snprintf vsnprintf], [], [])

    dnl AC_CHECK_HEADERS([librsync/prototab.h librsync/buf.h librsync/checksum.h librsync/command.h librsync/emit.h librsync/fileutil.h librsync/job.h librsync/mdfour.h librsync/netint.h librsync/protocol.h librsync/librsync.h librsync/librsync-config.h librsync/rollsum.h librsync/search.h librsync/stream.h librsync/sumset.h librsync/trace.h librsync/types.h librsync/util.h librsync/whole.h librsync/snprintf.h], [], [])
  fi
  
  dnl # --with-rsync -> add include path
  PHP_ADD_INCLUDE($RSYNC_DIR/include)

  dnl # --with-rsync -> check for lib and symbol presence
  LIBNAME=rsync # you may want to change this
  LIBSYMBOL=rs_librsync_version # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $RSYNC_DIR/lib, RSYNC_SHARED_LIBADD)
    AC_DEFINE(HAVE_RSYNCLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong librsync version or lib not found])
  ],[
    -L$RSYNC_DIR/lib -lm
  ])
  dnl
  PHP_SUBST(RSYNC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rsync, rsync.c, $ext_shared)
fi
