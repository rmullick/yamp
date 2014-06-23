//#include <sys/socket.h>
//include <netinet/in.h>
#include "thread.h"
#include "sockint.h"
#include <errno.h>

static int startport = 7000, endport = 7010, freeport = 7000;

void release_port(char *port)
{
	freeport = atoi(port);
	fprintf(stdout,"Port %d released\n", freeport);
}

int get_port(void)
{
	freeport++;
	return freeport-1;
}

inline void handle_command(int tfd)
{
	int l = sizeof(struct sockaddr_in), len = 0, port, repeat = 0;
	struct sockaddr_in taddr;
	char buf[15] = {0}, newbuf[15] = "port:";

	len = recvfrom(tfd, buf, 15, MSG_WAITALL, (struct sockaddr*)&taddr, (socklen_t *) &l);
	if (len <= 0)
		return;
	//fprintf(stdout,"Got data: of length: %d from port: %u addr: %s\n", len, taddr.sin_port, inet_ntoa(taddr.sin_addr));

	if (len > 0) {
		int pass = 0;
		buf[len] = 0;

		if (buf[0] == 'D' || buf[0] == 'd') {
			release_port(&buf[4]);
			return;
		}

		if (buf[0] == 'G' || buf[0] == 'g') {
			port = get_port();
			sprintf(&newbuf[5],"%d", port);
			newbuf[9] = 0; pass = 1;
		} else
			newbuf[0] = 0;

		repeat = 0;
		if (pass) {
			do {
				len = sendto(tfd, "port:7000 ", 10, MSG_DONTWAIT, (const struct sockaddr*)&taddr, (socklen_t) l);
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
	/*sigset_t set;
	sigemptyset(&set);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	sigaddset(&set, SIGUSR1);*/

	for(;;) {
		ret = poll(&tpfd, 1, -1);
		if (ret <= 0)
			continue;
		else {
			if (tpfd.revents & POLLIN) {
				//if (!pthread_mutex_trylock(&helper_mutex)) {
					handle_command(daemonfd);
					//pthread_mutex_unlock(&helper_mutex);
				//}
			}
		}
		/*pthread_sigmask(SIG_BLOCK, &set, NULL);
		ret = sigwait(&set, &signal);
		if (ret)
			continue;

		if (signal == SIGUSR1) {
			if (!pthread_mutex_trylock(&helper_mutex)) {
				handle_command();
				pthread_mutex_unlock(&helper_mutex);
			}
		}*/
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

