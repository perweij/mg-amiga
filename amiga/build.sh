#!/bin/sh
# Build mg for AmigaOS 3.x / 68000 with m68k-amigaos-gcc (AmigaPorts) and
# clib2; the result is amiga/out/mg.  Run from the top of the repository,
# with the cross compiler on the PATH.
#
#   MG_AMIGA_VERSION  version for the $VER string (default: 0.0.0-dev)
#
# Left out because clib2 has no fork/exec, regex or fnmatch: compile,
# cscope, ctags, dired (it runs ls), regex search, autoexec.  The
# ac_cv_func_* answers keep the pty and futimens replacements in lib/ out;
# src/amiga/compat.c has stubs for them.
set -eu
cd "$(dirname "$0")/.."

: "${MG_AMIGA_VERSION:=0.0.0-dev}"
date=$(date -u ${SOURCE_DATE_EPOCH:+-d @$SOURCE_DATE_EPOCH} +%-d.%-m.%Y)

# a configure without its helper scripts (left over by a git clean) too
[ -x configure ] && [ -f build-aux/install-sh ] || ./autogen.sh
./configure --host=m68k-amigaos --without-curses --without-docs \
    --disable-compile --disable-cscope --disable-ctags --disable-dired \
    --disable-regexp --disable-autoexec \
    ac_cv_func_openpty=yes ac_cv_func_login_tty=yes \
    ac_cv_func_fparseln=yes ac_cv_func_futimens=yes \
    CC="m68k-amigaos-gcc -mcrt=clib2" \
    CFLAGS="-m68000 -Os -DMG_AMIGA_VERSION='\"$MG_AMIGA_VERSION\"' -DMG_AMIGA_DATE='\"$date\"'"
make clean >/dev/null
make
mkdir -p amiga/out
m68k-amigaos-strip -o amiga/out/mg src/mg
ls -l amiga/out/mg
