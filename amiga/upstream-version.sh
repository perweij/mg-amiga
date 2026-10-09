#!/bin/sh
# The mg release this tree is based on, from configure.ac: "4.2".
sed -n 's/^AC_INIT(\[Mg\], \[\([^]]*\)\].*/\1/p' "$(dirname "$0")/../configure.ac"
