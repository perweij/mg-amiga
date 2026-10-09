/* <langinfo.h> for AmigaOS: newlib's (under clib2) pulls in a locale_t
 * that clashes with clib2's.  AmigaOS text is ISO-8859-1, so mg stays
 * out of UTF-8 mode.  Public domain, like mg. */
#ifndef MG_AMIGA_LANGINFO_H
#define MG_AMIGA_LANGINFO_H

typedef int	nl_item;
#define CODESET	0

static inline char *
nl_langinfo(nl_item item)
{
	return (item == CODESET ? "ISO-8859-1" : "");
}

#endif
