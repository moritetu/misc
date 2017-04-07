#!/bin/sh
#
# An utility script to setup the autoconf environment for the first time.
#
#
aclocal

automake -ac

# touch NEWS README AUTHORS ChangeLog

# Run autoconf (will create the 'configure'-script).
autoconf

echo 'Ready to go (run configure)'
