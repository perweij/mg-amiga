/*
 * AmigaOS 3.x compatibility, put in the public domain like mg.
 *
 * The AmigaOS build includes this file first in every source file
 * (-include, see src/Makefile.am), so that mg's POSIX code compiles
 * against clib2 mostly unchanged.  Calls that cannot work on AmigaOS
 * (fork, pipe, pty) fail at run time with ENOSYS, and mg reports that as
 * it would any failed call.  The Amiga-specific code is in this
 * directory; the few changes in mg's own files are marked __amigaos__.
 */
#ifndef MG_AMIGA_H
#define MG_AMIGA_H

/* With _GNU_SOURCE, newlib's headers (which clib2 includes) and clib2's
 * <locale.h> both define locale_t. */
#undef _GNU_SOURCE

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*
 * stat(): "" names nothing, as in POSIX; on AmigaOS it is the current
 * directory, and the unnamed *scratch* buffer then looked like a file
 * changed on disk.  clib2 has whole seconds only; mg wants timespecs.
 * The object-like macro renames both the function and the struct tag.
 */
struct mg_stat {
	mode_t		st_mode;
	ino_t		st_ino;
	dev_t		st_dev;
	dev_t		st_rdev;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	off_t		st_size;
	struct timespec	st_atim;
	struct timespec	st_mtim;
	struct timespec	st_ctim;
	long		st_blksize;
	long		st_blocks;
};
int	mg_stat(const char *, struct mg_stat *);
int	mg_lstat(const char *, struct mg_stat *);
int	mg_fstat(int, struct mg_stat *);
int	mg_access(const char *, int);
#define stat		mg_stat
#define lstat		mg_lstat
#define fstat		mg_fstat
#define access(p, m)	mg_access(p, m)

/*
 * Files keep their protection bits (hsparwed) and comment when mg writes
 * them.  clib2 recreates a file opened with O_CREAT|O_TRUNC and clears
 * the e bits of any file it created on close(); mg_open() opens existing
 * files in place and has AmigaOS create new ones (----rwed), and
 * mg_fchmod() sets r, w, e, d from the mode and leaves h, s, p, a alone.
 * rename() replaces an existing file, as in POSIX (clib2's fails), so
 * that a second session can replace the "file~" backup.
 */
#include <fcntl.h>
#include <stdio.h>
int	mg_open(const char *, int, ...);
int	mg_fchmod(int, mode_t);
int	mg_rename(const char *, const char *);
#define open		mg_open
#define fchmod		mg_fchmod
#define rename		mg_rename

/* AmigaOS text is ISO-8859-1: 0xa0-0xff are printable. */
#undef isprint
#define isprint(c)	(((c) >= 0x20 && (c) < 0x7f) || ((c) >= 0xa0 && (c) <= 0xff))

/* Window size from the console (Window Status Request), see console.c. */
struct winsize {
	unsigned short	ws_row, ws_col, ws_xpixel, ws_ypixel;
};
#ifndef TIOCGWINSZ
#define TIOCGWINSZ	0x40087468
#endif
int	mg_ioctl(int, unsigned long, void *);
#define ioctl(fd, req, arg)	mg_ioctl(fd, req, arg)

/* Signals AmigaOS has not: accepted and ignored. */
#ifndef SIGWINCH
#define SIGWINCH	28
#endif
#ifndef SIGCONT
#define SIGCONT		19
#endif
#ifndef SIGTSTP
#define SIGTSTP		18
#endif
#ifndef SIGCHLD
#define SIGCHLD		20
#endif
#ifndef SIGPIPE
#define SIGPIPE		13
#endif
struct sigaction {
	void		(*sa_handler)(int);
	sigset_t	sa_mask;
	int		sa_flags;
};
int	sigaction(int, const struct sigaction *, struct sigaction *);

/* termios and socket names clib2 does not have. */
#ifndef IMAXBEL
#define IMAXBEL		0
#endif
#ifndef SO_NOSIGPIPE
#define SO_NOSIGPIPE	0
#endif
#ifndef SHUT_RD
#define SHUT_RD		0
#define SHUT_WR		1
#define SHUT_RDWR	2
#endif

/* No processes to fork, no pipes, no ptys: these fail with ENOSYS. */
pid_t	fork(void);
int	pipe(int [2]);
pid_t	waitpid(pid_t, int *, int);
int	socketpair(int, int, int, int [2]);
int	openpty(int *, int *, char *, struct termios *, struct winsize *);
int	login_tty(int);

/* In clib2's headers, or missing from them, but not in its libc. */
size_t	strnlen(const char *, size_t);
char	*strndup(const char *, size_t);
long long strtoll(const char *, char **, int);
int	getpagesize(void);
int	futimens(int, const struct timespec [2]);	/* a no-op */

/* The shell window: raw console I/O in ttyio.c, the rest in console.c. */
int	amiga_setupterm(void *, int);
void	amiga_winsize(int *, int *);

/*
 * Path names are "Volume:dir/file": ':' ends a volume or an assign, '/'
 * separates directories, a leading or doubled '/' is the parent and ""
 * is the current directory.  A '/' is only appended to a non-empty name
 * that ends in neither ':' nor '/'.  See path.c.
 */
#define DIRSEP_NEEDED(s) ((s)[0] != '\0' && (s)[strlen(s) - 1] != '/' && \
			  (s)[strlen(s) - 1] != ':')
char	*mg_lastsep(const char *);
size_t	amiga_xdirname(char *, const char *, size_t);
size_t	amiga_xbasename(char *, const char *, size_t);
char	*amiga_adjustname(const char *, int);

#endif /* MG_AMIGA_H */
