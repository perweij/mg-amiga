/*
 * AmigaOS terminal I/O for mg, in place of ../ttyio.c.  Put in the public
 * domain, like mg.
 *
 * The Shell's console in raw mode (SetMode()), Read()/Write() on its
 * handles and WaitForChar() for polling.  The console sends CSI (0x9b)
 * where a VT100 sends ESC [, Backspace as ^H and Del as DEL; ttgetc()
 * hands mg what its ANSI keymap expects: ESC [, DEL (delete-backward-char)
 * and ^D (delete-char; ESC [ 3 ~ would be F4 here).
 */
#include <dos.h>		/* clib2: __check_abort_enabled, __stack_size */
#include <stdio.h>

#include <proto/dos.h>

#include "../def.h"

#define NOBUF	512			/* Output buffer size. */

int	ttstarted;
char	obuf[NOBUF];			/* Output buffer. */
size_t	nobuf;				/* Buffer count. */
int	nrow;				/* Terminal size, rows. */
int	ncol;				/* Terminal size, columns. */

/*
 * A Shell starts programs with 4 KB of stack, and mg keeps several 1 KB
 * path buffers on it (NFILEN); clib2's startup code gives more if asked.
 */
unsigned int	__stack_size = 65536;

static BPTR	ttin, ttout;
static char	pend[4];		/* Rewritten bytes not yet returned. */
static int	npend;

void
ttopen(void)
{
	/* ^C is an editor key here; clib2 would otherwise exit on it and
	 * leave the console in raw mode. */
	__check_abort_enabled = FALSE;
	ttin = Input();
	ttout = Output();
	if (!IsInteractive(ttin) || !IsInteractive(ttout))
		panic("standard input and output must be a console");

	if (ttraw() == FALSE)
		panic("aborting due to terminal initialize failure");
}

int
ttraw(void)
{
	if (!SetMode(ttin, 1)) {
		dobeep();
		ewprintf("ttopen can't set raw mode");
		return (FALSE);
	}
	/* No wrap at the right edge, no scroll at the bottom: mg writes
	 * the last column and the last line itself.  ttcooked() undoes it. */
	if (Write(ttout, "\x9b?7l\x9b>1l", 8) != 8)
		return (FALSE);
	ttstarted = 1;

	return (TRUE);
}

void
ttclose(void)
{
	if (ttstarted) {
		if (ttcooked() == FALSE)
			panic("");
		ttstarted = 0;
	}
}

/* Also the way back after a panic: the console defaults again. */
int
ttcooked(void)
{
	ttflush();
	(void)Write(ttout, "\x9b?7h\x9b>1h", 8);
	SetMode(ttin, 0);
	return (TRUE);
}

int
ttputc(int c)
{
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
	return (c);
}

/* ISO-8859-1: a cell is one byte. */
int
ttputcell(int cp)
{
	if (cp == 0)
		return (0);
	ttputc(cp);
	return (1);
}

void
ttflush(void)
{
	if (nobuf == 0 || batch == 1)
		return;
	if (Write(ttout, obuf, nobuf) != (LONG)nobuf)
		panic("ttflush write failed");
	nobuf = 0;
}

int
ttgetc(void)
{
	unsigned char	c;
	LONG		ret;
	int		i;

	if (npend > 0) {
		c = pend[0];
		for (i = 1; i < npend; i++)
			pend[i - 1] = pend[i];
		npend--;
		return (c);
	}
	do {
		ret = Read(ttin, &c, 1);
		if (ret == -1)
			panic("lost stdin");
	} while (ret != 1);

	switch (c) {
	case 0x9b:
		pend[0] = '[';
		npend = 1;
		return (0x1b);
	case 0x08:
		return (0x7f);
	case 0x7f:
		return (0x04);
	}
	return (c);
}

int
charswaiting(void)
{
	return (npend > 0 || WaitForChar(ttin, 0));
}

void
panic(char *s)
{
	static int panicking = 0;

	if (panicking)
		return;
	else
		panicking = 1;
	ttclose();
	(void) fputs("panic: ", stderr);
	(void) fputs(s, stderr);
	(void) fputc('\n', stderr);
	exit(1);
}

int
ttwait(int msec)
{
	if (npend > 0)
		return (FALSE);
	return (WaitForChar(ttin, (LONG)msec * 1000) ? FALSE : TRUE);
}
