/*
 * AmigaOS path names for mg: "Volume:dir/file".  ':' ends a volume or an
 * assign, '/' separates directories, a leading or doubled '/' means the
 * parent and "" is the current directory.  Put in the public domain.
 */
#include "../def.h"

/* The last separator, '/' or ':', or NULL. */
char *
mg_lastsep(const char *s)
{
	char	*slash = strrchr(s, '/'), *colon = strrchr(s, ':');

	return (slash > colon ? slash : colon);
}

/* "Vol:dir/f" -> "Vol:dir", "Vol:f" -> "Vol:", "f" -> "" */
size_t
amiga_xdirname(char *dp, const char *path, size_t dplen)
{
	char	 ts[NFILEN], *sep;

	(void)strlcpy(ts, path, sizeof(ts));
	if ((sep = mg_lastsep(ts)) == NULL)
		ts[0] = '\0';
	else
		sep[*sep == ':' ? 1 : 0] = '\0';
	return (strlcpy(dp, ts, dplen));
}

size_t
amiga_xbasename(char *bp, const char *path, size_t bplen)
{
	const char	*sep = mg_lastsep(path);

	return (strlcpy(bp, sep != NULL ? sep + 1 : path, bplen));
}

/*
 * No "//" rewriting and no realpath(): AmigaDOS resolves names itself.
 * A volume or assign typed after the prompt's default directory starts
 * the name over, as "//" does on Unix: "Work:dir/RAM:x" is "RAM:x".
 * A relative name gets the current directory, so buffers and prompts
 * show where the file is.
 */
char *
amiga_adjustname(const char *fn, int slashslash)
{
	static char	 fnb[NFILEN];
	const char	*cp, *ep;
	char		*path;

	if (slashslash == TRUE && (cp = strrchr(fn, ':')) != NULL) {
		for (ep = cp; ep > fn && ep[-1] != ':' && ep[-1] != '/'; ep--)
			;
		fn = ep;
	}
	if ((path = expandtilde(fn)) == NULL)
		return (NULL);
	fnb[0] = '\0';
	if (strchr(path, ':') == NULL && getcwd(fnb, sizeof(fnb)) != NULL &&
	    DIRSEP_NEEDED(fnb))
		(void)strlcat(fnb, "/", sizeof(fnb));
	(void)strlcat(fnb, path, sizeof(fnb));
	free(path);
	return (fnb);
}
