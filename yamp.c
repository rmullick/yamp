/*
 * Meant to be a simple mediaproxy.
 * Developed by Md. Rakib Hassan Mullick <rakib.mullick@gmail.com>
 * Licensed under GPLv2 or later.
 */

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include "sockint.h"
#include "thread.h"
#include "rtp.h"

void show_usages(void)
{
	fprintf(stderr, "Usages: ./progname -s startport -e endport\n");
	exit(1);
}

static void checklimit(int nr)
{
	struct rlimit resource;

	getrlimit(RLIMIT_NOFILE, &resource);
	if (resource.rlim_cur <= nr) {
		fprintf(stderr,"Try to increase resource limit. Check ulimit -n.\n");
		exit(1);
	}
}

int *allocfds(int nr)
{
	void *tmp;

	tmp = calloc(nr, sizeof(int));
	if (!tmp) {
		fprintf(stderr,"Failed to allocate resources.\n");
		exit(1);
	} else
		return tmp;
}

struct pollfd *allocpfdp(int nr)
{
	struct pollfd *tmp;
	tmp = calloc(nr, sizeof(struct pollfd));
	return tmp;
}

void initports(int nr)
{
	int x;

	freeports = calloc(nr, sizeof(int));
	if (!freeports) {
		fprintf(stderr,"Failed to alloc free port table\n");
		exit(1);
	}

	for (x = 0; x < nr; x++)
		freeports[x] = startport + x;

	freeidx = 0;
}


void inthandler(int sig)
{
	/* Free up the allocated memory */
	free(fdp);
	free(pfdp);
	free(sinfo);
	free(freeports);
	exit(1);
}

static void inline update_sinfo(const int x, const struct sockaddr_in *tmpclient, socklen_t l, unsigned char *buf, int size)
{
	unsigned short port1, port2;
	port1 = sinfo[x].caddr[0].sin_port;
	port2 = sinfo[x].caddr[1].sin_port;

	if (sinfo[x].flags == 2) {
		if ((port1 == tmpclient->sin_port) && (sinfo[x].caddr[0].sin_addr.s_addr == tmpclient->sin_addr.s_addr)) {
			size = sendto(fdp[x], buf, size, 0, (const struct sockaddr*)&sinfo[x].caddr[1], l);
			if (size < 1)
				printf("Failed to sent client[1]:%d\n", errno);
			return;
		}

		if ((port2 == tmpclient->sin_port) && (sinfo[x].caddr[1].sin_addr.s_addr == tmpclient->sin_addr.s_addr)) {
			size = sendto(fdp[x], buf, size, 0, (const struct sockaddr*)&sinfo[x].caddr[0], l);
			if (size < 1)
				printf("Failed to sent client[0]:%d\n", errno);
			return;
		}
	}

	if (sinfo[x].flags == 0) {
		if (!check_rtp(buf))
			return;

		sinfo[x].caddr[0].sin_family = tmpclient->sin_family;
		sinfo[x].caddr[0].sin_port = tmpclient->sin_port;		// binding port
		sinfo[x].caddr[0].sin_addr.s_addr = tmpclient->sin_addr.s_addr;	// interface
		sinfo[x].flags++;
		return;
	}

	if (sinfo[x].flags == 1) {
		if ((tmpclient->sin_port == port1) && (tmpclient->sin_addr.s_addr == sinfo[x].caddr[0].sin_addr.s_addr))
			return;

		if (!check_rtp(buf))
			return;

		sinfo[x].caddr[1].sin_family = tmpclient->sin_family;
		sinfo[x].caddr[1].sin_port = tmpclient->sin_port;		// binding port
		sinfo[x].caddr[1].sin_addr.s_addr = tmpclient->sin_addr.s_addr;	// interface
		sinfo[x].flags++;
		return;
	}
}

int main(int argc, char *argv[])
{
	int ret, l = sizeof(struct sockaddr), i, diff = 0, opt;

	if (argc != 5) {
		show_usages();
		exit(1);
	}

	startport = 0; endport = freeidx = active = portrange = 0;

	while ((opt = getopt(argc, argv, "e:s:")) != -1) {
		switch (opt) {
			case 's':
				startport = atoi(optarg);
				break;
			case 'e':
				endport = atoi(optarg);
				break;
			default:
				show_usages();
				break;
		}
	}

	if (endport >= startport) 
		diff = (endport - startport);
	else
		show_usages();

	checklimit(diff);

	fdp = allocfds(diff);
	pfdp = allocpfdp(diff);
	if (!pfdp) {
		free(fdp);
		fprintf(stderr, "Failed to allocate pollfds\n");
		exit(1);
	}

	memset(fdp, 0, sizeof(*fdp));
	memset(pfdp, 0, sizeof(*pfdp));

	sinfo = calloc(diff, sizeof(struct servinfo));
	if (!sinfo) {
		free(fdp); free(pfdp);
		exit(1);
	}

	initports(diff);

	if (prepare_helper()) {
		fprintf(stderr, "Failed to create daemon thread\n");
		inthandler(1);
		exit(1);
	}

	for (i = 0 ; i < diff; i++) {
		if (udp_open(&fdp[i], &sinfo[i].saddr, startport+i) == -1)
			continue;
		memset(&sinfo[i].caddr[0], 0, sizeof(struct sockaddr_in));
		memset(&sinfo[i].caddr[1], 0, sizeof(struct sockaddr_in));
		sinfo[i].flags = 0; sinfo[i].tcount = 0;
		pfdp[i].fd = fdp[i]; pfdp[i].events = POLLIN;
	}

	/* Install the signal handlers */
	signal(SIGINT, inthandler);
	signal(SIGTERM, inthandler);
	signal(SIGQUIT, inthandler);

	portrange = endport - startport;

	printf("Listening on %u.\n", portrange);

	while(1) {
		int len = 0, x = 0;
		ret = poll(pfdp, diff, 1000);
		if (ret < 0)
			continue;
		else {
		      for (x = 0; x < diff; x++) {
			unsigned char buf[BUFSIZE] = {0};
			struct sockaddr_in tmpclient = {.sin_family = AF_INET, .sin_port = 0, .sin_addr.s_addr = 0};
			if (pfdp[x].revents & POLLIN) {
				len = recvfrom(fdp[x], buf, BUFSIZE, 0, (struct sockaddr*)&tmpclient, (socklen_t*)&l);
				if (len > 0) {
					update_sinfo(x, &tmpclient, l, buf, len);
					sinfo[x].tcount = 0;
				}
			}

			if (!(x % 2) && sinfo[x].flags == 2) {
				if (sinfo[x].tcount < TIMEOUT)
					sinfo[x].tcount++;
				else {	/* Make this port available */
					int p;
					char port[10] = {0};
					p = ntohs(sinfo[x].saddr.sin_port);
					sprintf(port, "%d", p);
					release_port(port);
				}
			  }
		     }
		}
	}
}
