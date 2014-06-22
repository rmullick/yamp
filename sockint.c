#include "sockint.h"
#include <errno.h>

void nonblock(int *fd)
{
	int flags;
	flags = fcntl(*fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(*fd, F_SETFL, flags);
}

/* Helper for opening nonblocking udp socket.
 * @*fd - fd variable
 * @addr - sockaddr 
 * @port - where to bind
 */
void udp_open(int *fd, struct sockaddr_in *addr, int port)
{
	int yes = 0;

	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = INADDR_ANY;

	*fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (*fd < 0) {
		fprintf(stderr,"Failed to create socket, errno: %d\n", errno);
		return;
	}

	nonblock(fd);

	if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
		fprintf(stderr,"setsockopt() failed.\n");
		close(*fd);
		return;
	}

	if (bind(*fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr,"bind() failed %d.\n", errno);
		close(*fd);
		return;
	}
}
