#!/bin/sh
# Pack a built amiga/out/mg into a release archive for GitHub and Aminet:
#   amiga/out/mg-amiga-<version>-mg<upstream>.lha  (drawer mg-amiga/)
#   amiga/out/mg-amiga-<version>-mg<upstream>.readme  (Aminet readme)
# Run after amiga/build.sh, with the same MG_AMIGA_VERSION; needs lha
# (LHa for UNIX, in the AmigaPorts toolchain).
set -eu
cd "$(dirname "$0")/.."

: "${MG_AMIGA_VERSION:=0.0.0-dev}"
: "${MG_AMIGA_UPLOADER:=Per Weijnitz}"
upstream=$(amiga/upstream-version.sh)
name="mg-amiga-$MG_AMIGA_VERSION-mg$upstream"
out=amiga/out
stage=$out/stage/mg-amiga

[ -f $out/mg ] || { echo "no $out/mg; run amiga/build.sh first" >&2; exit 1; }
rm -rf $out/stage $out/$name.lha $out/$name.readme
mkdir -p $stage

cp $out/mg $stage/mg
subst() {
	sed -e "s/@VERSION@/$MG_AMIGA_VERSION/g" -e "s/@UPSTREAM@/$upstream/g" \
	    -e "s/@UPLOADER@/$MG_AMIGA_UPLOADER/g" "$1"
}
subst amiga/README.txt.in > $stage/README.txt
subst amiga/mg-amiga.readme.in > $out/$name.readme
cp $out/$name.readme $stage/mg-amiga.readme

# Licences of everything in the binary: mg (public domain), the lib/
# replacements this build linked, and clib2.
{
	echo "mg-amiga $MG_AMIGA_VERSION is mg $upstream for AmigaOS.  It contains:"
	echo
	echo "== mg, and the Amiga port: public domain (UNLICENSE)"
	echo
	cat UNLICENSE
	for o in $(sed -n 's/^LIBOBJS = //p' src/Makefile | tr -d '{}$U' | sed 's/LIBOBJDIR//g'); do
		c=lib/$(basename "$o" .o).c
		echo
		echo "== $c"
		echo
		sed -n '1,/\*\//p' "$c"
	done
	echo
	echo "== clib2, the C library (statically linked)"
	echo
	cat amiga/clib2.LICENSE
	echo
	echo "== libgcc: GPL with the GCC Runtime Library Exception, which puts"
	echo "   no conditions on programs compiled with it."
} > $stage/LICENSES.txt

(cd $out/stage && lha aq ../$name.lha mg-amiga)
rm -rf $out/stage
ls -l $out/$name.lha $out/$name.readme
