/*
 * The version string AmigaDOS's Version command finds in the binary:
 * "mg-amiga 1.2.0 (9.10.2026) mg 4.2", the port's version, the build
 * date and the mg release it is based on.  amiga/build.sh passes the
 * first two.  Put in the public domain, like mg.
 */
#include "config.h"

#ifndef MG_AMIGA_VERSION
#define MG_AMIGA_VERSION	"0.0.0-dev"
#endif
#ifndef MG_AMIGA_DATE
#define MG_AMIGA_DATE		"1.1.1978"
#endif

__attribute__((used)) const char mg_amiga_ver[] =
    "$VER: mg-amiga " MG_AMIGA_VERSION " (" MG_AMIGA_DATE ") mg " PACKAGE_VERSION;
