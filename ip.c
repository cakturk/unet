#include <stdio.h>
#include <stddef.h>	/* NULL */
#include "netif.h"
#include "mbuf.h"
#include "udp.h"
#include "netsniff.h"
#include "checksum.h"

static char fmt_buf[1024];

static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf
};

void
ip_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr *iph;

	iph = mb_head(m);

	if (ip_csum(iph, ip_hdrlen(iph), 0x0000) != 0) {
		fprintf(stderr, "IP: bad checksum\n");
		mb_free(m);
		return;
	}

	sb_reset(&sb);
	if (iphdr_print(iph, &sb) == 0)
		printf("ip: %s\n", sb.buf);

	/* IP proto demux */
	switch (iph->protocol) {
	case IPPROTO_UDP:
		udp_input(ifp, m);
		break;
	case IPPROTO_TCP:
		printf("TCP\n");
		break;
	}
}
