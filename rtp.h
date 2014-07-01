#ifndef	RTP_H
#define	RTP_H

struct rtp_hdr {
#if BYTE_ORDER == BIG_ENDIAN
    unsigned int version:2;     /* protocol version */
    unsigned int p:1;           /* padding flag */
    unsigned int x:1;           /* header extension flag */
    unsigned int cc:4;          /* CSRC count */
    unsigned int m:1;           /* marker bit */
    unsigned int pt:7;          /* payload type */
#else
    unsigned int cc:4;          /* CSRC count */
    unsigned int x:1;           /* header extension flag */
    unsigned int p:1;           /* padding flag */
    unsigned int version:2;     /* protocol version */
    unsigned int pt:7;          /* payload type */
    unsigned int m:1;           /* marker bit */
#endif
    unsigned int seq:16;        /* sequence number */
    unsigned ts;                /* timestamp */
    unsigned ssrc;              /* synchronization source */
    unsigned csrc[0];           /* optional CSRC list */
};

int check_rtp(const unsigned char *buf);

#endif
