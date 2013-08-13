lib_LTLIBRARIES = libmoodriver.la

INCLUDES = -I$(top_srcdir)/include $(MOO_CFLAGS) -DMOO_LOG_DOMAIN="\"MooDriver\""
libmoodriver_la_LDFLAGS = -export-dynamic -fPIC
libmoodriver_la_LIBADD = $(MOO_LIBS)
libmoodriver_la_SOURCES =   \
      md5.cc                \
      moo-driver.cc         \
      moo-io.cc             \
      util.cc               \
      xcommon.cc            \
      mucipher-aes.c        \
      mucipher-hexdigets.c  \
      mucipher-md5.c        \
      mucipher-sha256.c     \
      mucipher-sha.c        \
      mucipher-wraphelp.c

noinst_HEADERS =            \
      md5.hh                \
      moo-io.hh             \
      moo-types-private.hh  \
      util.hh               \
      xcommon.hh            

