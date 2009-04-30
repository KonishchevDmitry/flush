#!/bin/sh

# $Id: autotool.sh 2695 2008-09-20 16:52:56Z arvidn $

set -e
set -x

aclocal -I m4
libtoolize -c -f
automake -a -c -f
autoconf

rm -Rf config.cache autom4te.cache
