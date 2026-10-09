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
 * "." and ".." are the directory itself and its parent, as on Unix, for
 * dired ("^", the "." and ".." lines) and for "../x" in the prompts:
 * "Work:a/b/.." is "Work:a/", ".." at a volume's root stays there.
 * AmigaDOS has no such names (its parent is a leading or doubled '/'),
 * so a file really called ".." can't be reached through mg.  The name
 * has at most one ':' here.
 */
static void
dotdirs(char *path)
{
	char	*start, *rp, *wp, *seg, *q;
	size_t	 len;
	int	 trail;

	start = strchr(path, ':');
	start = start != NULL ? start + 1 : path;
	for (rp = wp = start; *rp != '\0'; ) {
		seg = rp;
		len = strcspn(rp, "/");
		rp += len;
		if ((trail = (*rp == '/')))
			rp++;
		if (len == 1 && seg[0] == '.')
			continue;
		if (len == 2 && seg[0] == '.' && seg[1] == '.') {
			if (wp == start)		/* at the root */
				continue;
			for (q = wp - 1; q > start && q[-1] != '/'; q--)
				;
			if (q == wp - 1)		/* after a "/" step */
				*wp++ = '/';
			else
				wp = q;
			continue;
		}
		memmove(wp, seg, len);
		wp += len;
		if (trail)
			*wp++ = '/';
	}
	*wp = '\0';
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
	dotdirs(fnb);
	return (fnb);
}
