bin_PROGRAMS = moo

moo_LDADD =   \
	$(MOO_LIBS) \
  $(top_builddir)/src/libmoodriver.la

moo_CXXFLAGS =            \
	-I$(top_srcdir)/include \
  $(MOO_CFLAGS)

moo_CFLAGS =        \
	-I$(top_srcdir)/src \
  $(MOO_CFLAGS)


moo_SOURCES = \
   moo.cc 
