/*
 * The Amiga console as mg's terminal: control sequences, keys, window
 * size, ISO-8859-1 letters.  Put in the public domain, like mg.
 */
#include "../def.h"
#include "../ansi.h"

/*
 * Called by setupterm() in ../ansi.c with its VT100 table: change what
 * differs on the Amiga console and read the window size.
 */
int
amiga_setupterm(void *term, int filedes)
{
	TERMINAL	*t = term;

	cur_term = t;
	t->t_fd = filedes;

	/* The console's insert/delete line, one line at a time. */
	t->t_str[22] = "\e[M";
	t->t_str[23] = "\e[L";
	t->t_str[46] = t->t_str[47] = t->t_str[48] = NULL;

	/*
	 * Keys: Shift+Left/Right = home/end, Shift+Up/Down = page up/down,
	 * F1-F10 = CSI 0~ .. CSI 9~, Help = CSI ?~ (bound like F1).
	 */
	t->t_str[49] = "\e[ A";
	t->t_str[52] = "\e[ @";
	t->t_str[53] = "\e[T";
	t->t_str[54] = "\e[S";
	t->t_str[55] = t->t_str[56] = t->t_str[57] = NULL;
	t->t_str[58] = "\e[?~";
	t->t_str[59] = "\e[1~";
	t->t_str[60] = "\e[2~";
	t->t_str[61] = "\e[3~";
	t->t_str[62] = "\e[4~";
	t->t_str[63] = "\e[5~";
	t->t_str[64] = "\e[6~";
	t->t_str[65] = "\e[7~";
	t->t_str[66] = "\e[8~";
	t->t_str[67] = "\e[9~";
	t->t_str[68] = t->t_str[69] = NULL;

	amiga_winsize(&t->t_nrow, &t->t_ncol);
	return (0);
}

/*
 * Window Status Request, CSI 0 SPC q, answered with CSI 1;1;<rows>;<cols>
 * SPC r; ttgetc() turns the CSI into ESC [.  Keys typed meanwhile are
 * lost; this runs at the start and on C-l.  Without an answer the size
 * stays as it was.
 */
void
amiga_winsize(int *rows, int *cols)
{
	int	n[4] = { 0, 0, 0, 0 }, i = 0, c, tries;

	ttputc(0x9b); ttputc('0'); ttputc(' '); ttputc('q');
	ttflush();
	for (tries = 0; tries < 64; tries++) {
		if (ttwait(500))
			return;
		c = ttgetc();
		if (c == 0x1b || c == '[') {
			i = 0;
			n[0] = n[1] = n[2] = n[3] = 0;
		} else if (c >= '0' && c <= '9' && i < 4)
			n[i] = n[i] * 10 + c - '0';
		else if (c == ';')
			i++;
		else if (c == 'r') {
			if (i == 3 && n[2] > 0 && n[3] > 0) {
				*rows = n[2];
				*cols = n[3];
			}
			return;
		}
	}
}

/* ISO-8859-1 letters (0xc0-0xff but the multiplication and division
 * signs) are word characters; mg's table only knows ASCII. */
__attribute__((constructor)) static void
latin1_words(void)
{
	int	c;

	for (c = 0xc0; c <= 0xff; c++)
		if (c != 0xd7 && c != 0xf7)
			cinfo[c] |= _MG_W;
}
