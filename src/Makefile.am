CLEANFILES =
BUILT_SOURCES =
AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)

# Tell the linker to omit references to unused shared libraries.

noinst_LIBRARIES = libver.a
nodist_libver_a_SOURCES = version.c version.h

AM_LDFLAGS = $(IGNORE_USED_LIBRARIES_CFLAGS)
gzip_LDADD = libver.a ../lib/libgzip.a
gzip_LDADD += $(LIB_CLOCK_GETTIME)

bin_PROGRAMS = gzip
gzip_SOURCES = \
  bits.c deflate.c gzip.c inflate.c lzw.c \
  trees.c unlzh.c unlzw.c unpack.c unzip.c util.c zip.c

if IBM_Z_DFLTCC
gzip_SOURCES += dfltcc.c
endif


BUILT_SOURCES += version.c
version.c: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf '#include <config.h>\n' > $@t
	$(AM_V_at)printf 'char const *Version = "$(PACKAGE_VERSION)";\n' >> $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

BUILT_SOURCES += version.h
version.h: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf 'extern char const *Version;\n' > $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@