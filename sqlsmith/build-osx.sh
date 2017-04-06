#!/usr/bin/env bash
#
# Build sqlsmith
#
_e() { echo "==> $@"; }

if [ -d sqlsmith ]; then
  /bin/rm -rf sqlsmith
fi

_e "install library"
brew install libpqxx automake libtool autoconf autoconf-archive


_e "clone sqlsmith"
git clone git@github.com:anse1/sqlsmith.git && cd sqlsmith
# change include base path
sed -i -e "s%^#include <postgresql/\(libpq-fe\.h\)>$%#include <\1>%g" postgres.hh
autoreconf -i

_e "build sqlsmith"
./configure --with-postgresql=$(brew --prefix)/opt/postgresql/bin/pg_config
make
