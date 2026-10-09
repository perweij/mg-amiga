/*
 * The directory listing for dired on AmigaOS, in place of "ls -al" (no
 * programs to run).  Put in the public domain, like mg.
 *
 * The lines have ls -al's layout, so that ../dired.c finds the name after
 * the 8th field and the type in column 2:
 *
 *   total 42
 *   d----rwed 1 0 0        0 Oct  9 12:00 .
 *   d----rwed 1 0 0        0 Oct  9 12:00 ..
 *   -----rw-d 1 0 0      628 Oct  9 12:00 apps.info
 *
 * The mode field is the type ('d', 'l' for a soft link, '-') and the
 * protection bits as List shows them (hsparwed), owner and group are
 * the FileInfoBlock's (0 at a volume's root, where they are not set),
 * total is the number of blocks.  "." and ".." are
 * the directory itself; path.c resolves them.  Names are sorted without
 * regard to case, like AmigaDOS compares them.
 */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>

#include "../def.h"

#define DAYS_1970_1978	2922		/* AmigaDOS dates count from 1978 */
#define RECENT_DAYS	183		/* ls: time, not year, if this recent */

struct dent {
	char		*line;
	const char	*name;		/* in line */
};

static char *
dentline(const struct FileInfoBlock *fib, const char *name,
    const struct DateStamp *now)
{
	static const char	 mon[12][4] = { "Jan", "Feb", "Mar", "Apr",
	    "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static const char	 bits[] = "hsparwed";
	const struct DateStamp	*ds = &fib->fib_Date;
	char			 mode[10], date[16], *line;
	struct tm		*tm;
	time_t			 t;
	size_t			 len;
	int			 i, set;

	mode[0] = fib->fib_DirEntryType == ST_SOFTLINK ? 'l' :
	    fib->fib_DirEntryType > 0 ? 'd' : '-';
	for (i = 0; i < 8; i++) {
		set = (fib->fib_Protection >> (7 - i)) & 1;
		if (i >= 4)		/* r, w, e, d are set when denied */
			set = !set;
		mode[i + 1] = set ? bits[i] : '-';
	}
	mode[9] = '\0';

	/* AmigaDOS dates are local time already: no zone to apply. */
	t = ((time_t)ds->ds_Days + DAYS_1970_1978) * 86400 +
	    ds->ds_Minute * 60 + ds->ds_Tick / TICKS_PER_SECOND;
	if ((tm = gmtime(&t)) == NULL)
		(void)strlcpy(date, "Jan  1  1978", sizeof(date));
	else if (ds->ds_Days <= now->ds_Days &&
	    ds->ds_Days > now->ds_Days - RECENT_DAYS)
		(void)snprintf(date, sizeof(date), "%s %2d %02d:%02d",
		    mon[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min);
	else
		(void)snprintf(date, sizeof(date), "%s %2d %5d",
		    mon[tm->tm_mon], tm->tm_mday, tm->tm_year + 1900);

	len = strlen(name) + 64;
	if ((line = malloc(len)) == NULL)
		return (NULL);
	(void)snprintf(line, len, "%s 1 %u %u %8ld %s %s", mode,
	    (unsigned)fib->fib_OwnerUID, (unsigned)fib->fib_OwnerGID,
	    fib->fib_DirEntryType > 0 ? 0L : (long)fib->fib_Size, date, name);
	return (line);
}

static int
dentcmp(const void *a, const void *b)
{
	return (strcasecmp(((const struct dent *)a)->name,
	    ((const struct dent *)b)->name));
}

static int
dentadd(struct dent **dp, size_t *np, size_t *maxp,
    const struct FileInfoBlock *fib, const char *name,
    const struct DateStamp *now)
{
	struct dent	*d;
	char		*line;

	if (*np == *maxp) {
		*maxp = *maxp ? *maxp * 2 : 64;
		if ((d = realloc(*dp, *maxp * sizeof(*d))) == NULL)
			return (FALSE);
		*dp = d;
	}
	if ((line = dentline(fib, name, now)) == NULL)
		return (FALSE);
	(*dp)[*np].line = line;
	(*dp)[*np].name = line + strlen(line) - strlen(name);
	(*np)++;
	return (TRUE);
}

/* Add the listing of directory dname to bp, as d_exec("ls -al") does. */
int
amiga_dirlist(struct buffer *bp, const char *dname)
{
	struct FileInfoBlock	*fib, dot;
	struct DateStamp	 now;
	struct stat		 sb;
	struct dent		*d = NULL;
	size_t			 n = 0, max = 0, i;
	long			 blocks = 0;
	BPTR			 lock = 0, parent;
	int			 ret = FALSE;

	if ((fib = AllocDosObject(DOS_FIB, NULL)) == NULL) {
		dobeep();
		ewprintf("Out of memory");
		return (FALSE);
	}
	if ((lock = Lock((STRPTR)dname, ACCESS_READ)) == 0 ||
	    !Examine(lock, fib) || fib->fib_DirEntryType <= 0) {
		dobeep();
		ewprintf("Can't read directory %s", dname);
		goto out;
	}
	DateStamp(&now);
	dot = *fib;
	if ((parent = ParentDir(lock)) == 0) {	/* a root: no bits, owner */
		dot.fib_Protection = 0;
		dot.fib_OwnerUID = dot.fib_OwnerGID = 0;
	} else
		UnLock(parent);
	if (!dentadd(&d, &n, &max, &dot, ".", &now) ||
	    !dentadd(&d, &n, &max, &dot, "..", &now))
		goto nomem;
	while (ExNext(lock, fib)) {
		blocks += fib->fib_NumBlocks;
		if (!dentadd(&d, &n, &max, fib,
		    (const char *)fib->fib_FileName, &now))
			goto nomem;
	}
	if (IoErr() != ERROR_NO_MORE_ENTRIES) {
		dobeep();
		ewprintf("Error reading directory %s", dname);
		goto out;
	}
	qsort(d + 2, n - 2, sizeof(*d), dentcmp);

	addlinef(bp, "  total %ld", blocks);
	for (i = 0; i < n; i++)
		addlinef(bp, "  %s", d[i].line);

	/*
	 * What fupdstat() gets on Unix, for fchecktime(); here it can't
	 * fopen() a directory.
	 */
	if (stat(dname, &sb) == 0) {
		bp->b_fi.fi_mode = sb.st_mode | 0x8000;
		bp->b_fi.fi_uid = sb.st_uid;
		bp->b_fi.fi_gid = sb.st_gid;
		bp->b_fi.fi_mtime = sb.st_mtim;
		bp->b_flag &= ~(BFIGNDIRTY | BFDIRTY);
	}
	ret = TRUE;
	goto out;
nomem:
	dobeep();
	ewprintf("Out of memory");
out:
	for (i = 0; i < n; i++)
		free(d[i].line);
	free(d);
	if (lock)
		UnLock(lock);
	FreeDosObject(DOS_FIB, fib);
	return (ret);
}
