/*
 * POSIX pieces that clib2 lacks or only declares, for AmigaOS.
 * Put in the public domain, like mg.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <pwd.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>

#include <proto/dos.h>

/* The real clib2 calls under the wrappers' names. */
#undef stat
#undef lstat
#undef fstat
#undef access
#undef ioctl

static void
copystat(struct mg_stat *to, const struct stat *from)
{
	memset(to, 0, sizeof(*to));
	to->st_mode = from->st_mode;
	to->st_ino = from->st_ino;
	to->st_dev = from->st_dev;
	to->st_rdev = from->st_rdev;
	to->st_nlink = from->st_nlink;
	to->st_uid = from->st_uid;
	to->st_gid = from->st_gid;
	to->st_size = from->st_size;
	to->st_atim.tv_sec = from->st_atime;
	to->st_mtim.tv_sec = from->st_mtime;
	to->st_ctim.tv_sec = from->st_ctime;
	to->st_blksize = from->st_blksize;
	to->st_blocks = from->st_blocks;
}

int
mg_stat(const char *path, struct mg_stat *sb)
{
	struct stat	st;

	if (path[0] == '\0') {
		errno = ENOENT;
		return (-1);
	}
	if (stat(path, &st) == -1)
		return (-1);
	copystat(sb, &st);
	return (0);
}

int
mg_lstat(const char *path, struct mg_stat *sb)
{
	struct stat	st;

	if (path[0] == '\0') {
		errno = ENOENT;
		return (-1);
	}
	if (lstat(path, &st) == -1)
		return (-1);
	copystat(sb, &st);
	return (0);
}

int
mg_fstat(int fd, struct mg_stat *sb)
{
	struct stat	st;

	if (fstat(fd, &st) == -1)
		return (-1);
	copystat(sb, &st);
	return (0);
}

int
mg_access(const char *path, int mode)
{
	if (path[0] == '\0') {
		errno = ENOENT;
		return (-1);
	}
	return (access(path, mode));
}

int
mg_ioctl(int fd, unsigned long req, void *arg)
{
	if (req == TIOCGWINSZ) {
		struct winsize	*ws = arg;
		int		 rows = 0, cols = 0;

		amiga_winsize(&rows, &cols);
		if (rows <= 0 || cols <= 0) {
			errno = ENOTTY;
			return (-1);
		}
		memset(ws, 0, sizeof(*ws));
		ws->ws_row = rows;
		ws->ws_col = cols;
		return (0);
	}
	return (ioctl(fd, req, arg));
}

/* Only the console can be polled: fd 0 is the Shell window's input. */
int
poll(struct pollfd *fds, nfds_t n, int timeout)
{
	if (n != 1 || fds[0].fd != 0) {
		errno = ENOSYS;
		return (-1);
	}
	fds[0].revents = 0;
	if (WaitForChar(Input(), timeout < 0 ? 0x7fffffff : (LONG)timeout * 1000)) {
		fds[0].revents = POLLIN;
		return (1);
	}
	return (0);
}

int
sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	if (oact != NULL)
		memset(oact, 0, sizeof(*oact));
	return (0);
}

pid_t
fork(void)
{
	errno = ENOSYS;
	return (-1);
}

int
pipe(int fd[2])
{
	errno = ENOSYS;
	return (-1);
}

pid_t
waitpid(pid_t pid, int *status, int options)
{
	errno = ECHILD;
	return (-1);
}

int
socketpair(int domain, int type, int protocol, int sv[2])
{
	errno = ENOSYS;
	return (-1);
}

/* Backup copies keep the time of their writing. */
int
futimens(int fd, const struct timespec times[2])
{
	return (0);
}

/* Declared by clib2, not in its libc: nothing to run programs with. */
int
execv(const char *path, char * const argv[])
{
	errno = ENOSYS;
	return (-1);
}

int
execlp(const char *file, const char *arg0, ...)
{
	errno = ENOSYS;
	return (-1);
}

ssize_t
send(int s, const void *buf, size_t len, int flags)
{
	errno = ENOSYS;
	return (-1);
}

int
shutdown(int s, int how)
{
	errno = ENOSYS;
	return (-1);
}

/* No regex library: the startup file's regex test reports an error. */
int
regcomp(regex_t *preg, const char *pattern, int cflags)
{
	return (REG_ESPACE);
}

int
regexec(const regex_t *preg, const char *string, size_t nmatch,
    regmatch_t pmatch[], int eflags)
{
	return (REG_NOMATCH);
}

void
regfree(regex_t *preg)
{
}

int
openpty(int *amaster, int *aslave, char *name, struct termios *termp,
    struct winsize *winp)
{
	errno = ENOSYS;
	return (-1);
}

int
login_tty(int fd)
{
	errno = ENOSYS;
	return (-1);
}

size_t
strnlen(const char *s, size_t n)
{
	size_t	 len = 0;

	while (len < n && s[len] != '\0')
		len++;
	return (len);
}

char *
strndup(const char *s, size_t n)
{
	size_t	 len = strnlen(s, n);
	char	*p = malloc(len + 1);

	if (p != NULL) {
		memcpy(p, s, len);
		p[len] = '\0';
	}
	return (p);
}

long long
strtoll(const char *s, char **end, int base)
{
	return (strtol(s, end, base));	/* long is 32 bits: enough for mg */
}

int
getpagesize(void)
{
	return (4096);		/* no MMU pages; mg only uses it as a size */
}

/* No user database: ~ is $HOME when that is set. */
uid_t
geteuid(void)
{
	return (0);
}

struct passwd *
getpwuid(uid_t uid)
{
	static struct passwd	 pw;
	char			*home = getenv("HOME");

	if (home == NULL || *home == '\0')
		return (NULL);
	memset(&pw, 0, sizeof(pw));
	pw.pw_name = "amiga";
	pw.pw_dir = home;
	return (&pw);
}

struct passwd *
getpwnam(const char *name)
{
	return (NULL);
}
