AM_CFLAGS = -Wundef \
	-Wstrict-prototypes \
	-Wno-trigraphs \
	-g -O0 \
	-fno-inline \
	-fno-short-enums \
	-fPIC \
	-Wall \
	-Wextra

requiredlibs = -ldl
library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = power.c metadata-parser.c utils.c list.c hint-data.c power-8974.c

lib_LTLIBRARIES = libpower.la
libpower_la_SOURCES = $(c_sources) $(h_sources)
libpower_la_CFLAGS = $(AM_CFLAGS)
libpower_la_LDFLAGS = -avoid-version
libpower_la_LIBADD = $(requiredlibs)

