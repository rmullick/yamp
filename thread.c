#include "thread.h"
#include "sockint.h"
#include <signal.h>

void release_port(char *port)
{
	int idx;

	/* as soon as some port is available that means we're ready to take calls */
	if (active > portrange)
		active = portrange-1;
	else
		active--;

	idx = 0;
	while ((freeports[idx] != -1) && idx <= portrange)
		idx++;

	/* this never shouldn't happen ! */
	if (idx == portrange && freeports[idx] != -1) {
		fprintf(stderr, "Failed to release port.\n");
		return 
	}

	freeports[idx] = atoi(port);
	freeidx = idx;			/* freeidx is a sortof cache-ing ports */

	fprintf(stdout,"Port %s released\n", port);
}

int get_port(void)
{
	int preidx, port;

	if (active >= portrange)
		return -1;
	else
		active++;

	preidx = freeidx;
	if (freeports[freeidx] != -1) {
		port = freeports[preidx];
		freeports[preidx] = -1;
		return port;
	}
	preidx = 0;
	while (freeports[preidx] == -1)
		preidx++;

	freeidx = preidx;

	port = freeports[preidx];
	freeports[preidx] = -1;		/* Not avaiable anymore */

	return port;
}

inline void handle_command(int tfd)
{
	int l = sizeof(struct sockaddr_in), len = 0, port, repeat = 0;
	struct sockaddr_in taddr;
	char buf[15] = {0}, newbuf[15] = "port:0000000000";

	len = recvfrom(tfd, buf, 15, 0, (struct sockaddr*)&taddr, (socklen_t *) &l);
	if (len <= 0)
		return;
	else {
		int pass = 0;
		buf[len] = 0;

		if (buf[0] == 'D' || buf[0] == 'd') {
			release_port(&buf[4]);
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
				if (len <= 0) {
					//fprintf(stdout,"failed to sent: %d\n", errno);
					repeat++;
				} else
					break;
			} while (repeat < 3);
		}
	}
}

static void *helper_routine(void *info)
{
	int signal, ret, daemonfd;
	struct pollfd tpfd;
	struct sockaddr_in daemonserv;

	if (udp_open(&daemonfd, &daemonserv, 4444) == -1) {
		fprintf(stderr,"Failed to create daemon. Exiting...\n");
		return;
	}

	memset(&tpfd, 0, sizeof(struct pollfd));
	tpfd.fd = daemonfd; tpfd.events = POLLIN;

	for(;;) {
		ret = poll(&tpfd, 1, -1);
		if (ret <= 0)
			continue;
		else {
			if (tpfd.revents & POLLIN) {
					handle_command(daemonfd);
			}
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
