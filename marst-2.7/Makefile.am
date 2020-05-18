# Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = marst macvt

marst_SOURCES = marst.c

macvt_SOURCES = macvt.c

include_HEADERS = algol.h

lib_LTLIBRARIES = libalgol.la

SUFFIXES = .alg

.alg.c:
	./marst -t -w $< -o $@

libalgol_la_SOURCES = alglib1.c alglib2.c alglib3.c

$(libalgol_la_SOURCES): marst$(EXEEXT)

check: marst$(EXEEXT) macvt$(EXEEXT) libalgol.la
	./macvt -i -s examples/euler.txt -o examples/euler.alg
	./marst examples/euler.alg -o examples/euler.c
	./marst examples/gps.alg -o examples/gps.c
	$(CC) $(CFLAGS) -I. examples/gps.c -o examples/gps $(LDFLAGS) \
		.libs/libalgol.a -lm
	./examples/gps
	@echo "If matrix C = A * B is unity, the check is OK"
