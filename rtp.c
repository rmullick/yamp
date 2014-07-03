#include "rtp.h"

/*
 * Checks whether @buf is a valid rtp packet or not.
 * Checking rtp packet is quite lame, not robust.
 * Here we're only checking for the rtp version, atm.
 */
int check_rtp(const unsigned char *buf)
{
	struct rtp_hdr *rtph;

	rtph = (struct rtp_hdr *) buf;

	if (rtph->version == 2)
		return 1;

	return 0;
}
