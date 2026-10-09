/* <err.h> for AmigaOS: clib2 has none.  Public domain, like mg. */
#ifndef MG_AMIGA_ERR_H
#define MG_AMIGA_ERR_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void
vwarn_(int with_errno, const char *fmt, va_list ap)
{
	int	 e = errno;

	fputs("mg: ", stderr);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	if (with_errno)
		fprintf(stderr, "%s%s", fmt != NULL ? ": " : "", strerror(e));
	fputc('\n', stderr);
}

static inline void
warn(const char *fmt, ...)
{
	va_list	 ap;

	va_start(ap, fmt);
	vwarn_(1, fmt, ap);
	va_end(ap);
}

static inline void
warnx(const char *fmt, ...)
{
	va_list	 ap;

	va_start(ap, fmt);
	vwarn_(0, fmt, ap);
	va_end(ap);
}

static inline void
err(int eval, const char *fmt, ...)
{
	va_list	 ap;

	va_start(ap, fmt);
	vwarn_(1, fmt, ap);
	va_end(ap);
	exit(eval);
}

static inline void
errx(int eval, const char *fmt, ...)
{
	va_list	 ap;

	va_start(ap, fmt);
	vwarn_(0, fmt, ap);
	va_end(ap);
	exit(eval);
}

#endif
