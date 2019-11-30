AC_INIT(main.c)

AC_PROG_CC
AC_LANG_C
AC_PROG_MAKE_SET
AC_HEADER_STDC
AC_SYS_LARGEFILE

AC_ARG_WITH(system-libuseful, [  --with-system-libuseful       use systemwide libUseful.so rather than bundled version], cf_with_libuseful=$withval )

if test "$cf_with_libuseful" != "no" -a "$cf_with_libuseful" != ""
then
AC_CHECK_LIB([Useful],[OpenSSLVerifyCallback],
  [AC_CHECK_LIB([ssl],[SSL_new],,,[-lcrypto -lz])]
  [AC_CHECK_LIB([crypto],[ERR_load_CRYPTO_strings],,,[-lz])]
  [AC_CHECK_LIB([z],[inflate])]
  ,,[-lssl -lcrypto -lz])
AC_CHECK_LIB(Useful,GetToken)

LUSEFUL="system"
else
LUSEFUL="bundled"
STATIC_LIBS="libUseful/libUseful.a"
AC_SUBST(STATIC_LIBS)
AC_CONFIG_SUBDIRS([libUseful])
fi


dnl read Makefile.in and write Makefile
AC_OUTPUT(Makefile)

