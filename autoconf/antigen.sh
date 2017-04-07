#!/bin/sh
#
# An utility script to remove all generated files.
#
if test -f Makefile; then
  make distclean
fi

rm -f *.tar.* *.tgz

rm -Rf autom4te.cache

rm -f Makefile.in aclocal.m4 configure depcomp install-sh missing compile
