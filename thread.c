/*
 * This is a helper thread, which does the talking accepts
 * requests from sip-servers and act accordingly, i.e get/release
 * ports.
 */

#include "thread.h"
#include "sockint.h"
#include <signal.h>
#include <stdlib.h>

/* Only for some debugging purposes, will be removed later */
void showfreeports(void)
{
	int i;

	for (i = 0; i < portrange; i++)
		fprintf(stdout,"%d\n", freeports[i]);
}

/*
 * Currently to port acquire/release is done via linear traversing over
 * the free port lists. So, if no of portrange is big, it might suffer.
 * Might need attention for that purpose.
 */
void release_port(int port)
{
	unsigned int idx, p;

	/* as soon as some port is available that means we're ready to take calls */
	if (active > portrange/2)
		active = portrange-1;
	else
		active--;
	
	p = port;
	idx = p - startport;	/* idx is index of port */
	fprintf(stdout,"Port %d:%d released\n", port, idx);

	freeports[idx] = p;
	sinfo[idx].tcount = 0;
	sinfo[idx].flags = 0;	/* reset the servinfo flags */
	freeidx = idx;		/* freeidx is a sortof cache-ing ports */
}

int get_port(void)
{
	int preidx, port;

	if (active >= portrange/2)
		return -1;
	else
		active++;

	preidx = freeidx;
	if (freeports[freeidx] != -1) {
		sinfo[preidx].tcount = 0;
		port = freeports[preidx];
		sinfo[preidx].tcount = 0;
		freeports[preidx] = -1;
		return port;
	}

	preidx = 0;
	while (freeports[preidx] == -1)
		preidx += 2;

	freeidx = preidx;

	port = freeports[preidx];
	freeports[preidx] = -1;	/* Not avaiable anymore */

	return port;
}

inline void handle_command(int tfd)
{
	int l = sizeof(struct sockaddr_in), len = 0, repeat = 0;
	struct sockaddr_in taddr;
	char buf[15] = {0}, newbuf[15] = "port:0000000000";

	len = recvfrom(tfd, buf, 15, 0, (struct sockaddr*)&taddr, (socklen_t *) &l);
	if (len <= 0)
		return;
	else {
		int pass = 0, port;
		buf[len] = 0;

		/* This is only for testing purpose, to be removed later */
		if (buf[0] == 'S') {
			showfreeports();
			return;
		}

		/* Current deletion format is: DEL:portno */
		if (buf[0] == 'D' || buf[0] == 'd') {
			port = atoi(&buf[4]);
			pthread_mutex_lock(&releaseport);
			release_port(port);
			pthread_mutex_unlock(&releaseport);
			return;
		}

		if (buf[0] == 'G' || buf[0] == 'g') {
			port = get_port();
			if (port != -1) {
				sprintf(&newbuf[5],"%d", port);
				pass = 1;
			}
		} else
			newbuf[0] = 0;

		repeat = 0;
		if (pass) {
			do {
				len = sendto(tfd, newbuf, 10, MSG_DONTWAIT, (const struct sockaddr*)&taddr, (socklen_t) l);
				if (len <= 0)
					repeat++;
				else
					break;
			} while (repeat < 3);
		}
	}
}

static void *helper_routine(void *info)
{
	int ret, daemonfd;
	struct pollfd tpfd;
	struct sockaddr_in daemonserv;

	if (udp_open(&daemonfd, &daemonserv, 4444) == -1) {
		fprintf(stderr,"Failed to create daemon. Exiting...\n");
		return NULL;
	}

	memset(&tpfd, 0, sizeof(struct pollfd));
	tpfd.fd = daemonfd; tpfd.events = POLLIN;

	for (;;) {
		ret = poll(&tpfd, 1, -1);
		if (ret <= 0)
			continue;
		else {
			if (tpfd.revents & POLLIN)
				handle_command(daemonfd);
		}
	}
}

int prepare_helper(void)
{
	pthread_attr_t attr;
	pthread_t tinfo;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&tinfo, &attr, &helper_routine, NULL))
		return -1;

	return 0;
}
