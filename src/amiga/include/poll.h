/* <poll.h> for AmigaOS: clib2 has none.  Only the console (fd 0) can be
 * polled, see compat.c.  Public domain, like mg. */
#ifndef MG_AMIGA_POLL_H
#define MG_AMIGA_POLL_H

#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

typedef unsigned int nfds_t;

struct pollfd {
	int	fd;
	short	events;
	short	revents;
};

int	poll(struct pollfd *, nfds_t, int);

#endif
