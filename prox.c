/*
 * Meant to be a simple mediaproxy.
 * Developed by Md. Rakib Hassan Mullick <rakib.mullick@gmail.com>
 * Released under GPLv2 or later.
 */

#include "sockint.h"
#include <poll.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#define	NRSERV	1024

struct servinfo {
	struct sockaddr_in saddr, caddr[2];
	int flags;
};

int main(void)
{
	int fd[10], ret, l = sizeof(struct sockaddr), i;
	struct servinfo serv[NRSERV];
	struct pollfd pfd[NRSERV];

	memset(&pfd, 0, sizeof(struct pollfd) * NRSERV);
	for (i = 0 ; i < NRSERV; i++) {
		if (udp_open(&fd[i], &serv[i].saddr, 10000+i) == -1)
			continue;
		memset(&serv[i].caddr[0], 0, sizeof(struct sockaddr_in));
		memset(&serv[i].caddr[1], 0, sizeof(struct sockaddr_in));
		serv[i].flags = 0;
		pfd[i].fd = fd[i]; pfd[i].events = POLLIN;
	}

	printf("Listening for req on 7000\n");

	while(1) {
		int len = 0, x = 0;
		char buf[100] = {0};
		struct sockaddr_in tmpclient;
		ret = poll(pfd, NRSERV, -1);
		if (ret <= 0)
			continue;
		else {
		      for (x = 0; x < 10; x++) {
			memset(buf, 0, 100);
			memset(&tmpclient, 0, sizeof(tmpclient));
			if (pfd[x].revents & POLLIN) {	// incoming data
				len = recvfrom(fd[x], buf, 100, 0, (struct sockaddr*)&tmpclient, (socklen_t*)&l);
				if (len > 0) {
					if (serv[x].flags == 0)	{
						serv[x].caddr[0].sin_family = tmpclient.sin_family;
						serv[x].caddr[0].sin_port = tmpclient.sin_port;		// binding port
						serv[x].caddr[0].sin_addr.s_addr = tmpclient.sin_addr.s_addr;	// interface
						serv[x].flags++;
						continue;
					}

					if (serv[x].flags == 1) {
					   if ((tmpclient.sin_port == serv[x].caddr[0].sin_port) && (tmpclient.sin_addr.s_addr == serv[x].caddr[0].sin_addr.s_addr))
						          continue;
						serv[x].caddr[1].sin_family = tmpclient.sin_family;
						serv[x].caddr[1].sin_port = tmpclient.sin_port;		// binding port
						serv[x].caddr[1].sin_addr.s_addr = tmpclient.sin_addr.s_addr;	// interface
						serv[x].flags++;
						continue;
					}

					if (serv[x].flags == 2) {
						//printf("both client found\n");
					   if ((serv[x].caddr[0].sin_port == tmpclient.sin_port) && (serv[x].caddr[0].sin_addr.s_addr == tmpclient.sin_addr.s_addr))
							len = sendto(fd[x], buf, len, MSG_DONTWAIT, (const struct sockaddr*)&serv[x].caddr[1], l);
							if (len < 1)
								printf("Failed to sent client[1]\n");
					   if ((serv[x].caddr[1].sin_port == tmpclient.sin_port) && (serv[x].caddr[1].sin_addr.s_addr == tmpclient.sin_addr.s_addr))
							len = sendto(fd[x], buf, len, MSG_DONTWAIT, (const struct sockaddr*)&serv[x].caddr[0], l);
							if (len < 1)
								printf("Failed to sent client[0]\n");
					}
				}
			  }
		     }
		}
	}
}
