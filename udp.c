#include <stdio.h>
#include "netsniff.h"
#include "mbuf.h"
#include "checksum.h"

struct netif;

void
udp_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr  *iph;
	struct udphdr *uh;

	iph = mb_head(m);
	mb_htrim(m, ip_hdrlen(iph));

	if (mb_datalen(m) < sizeof(*uh)) {
		fprintf(stderr, "UDP: packet too small for the header\n");
		goto drop;
	}

	uh = mb_head(m);
	if (ntohs(uh->len) < mb_datalen(m)) {
		fprintf(stderr, "UDP: packet too small for the payload\n");
		goto drop;
	}

	if (uh->csum && ip_udp_csum(iph->saddr, iph->daddr, uh) != 0) {
		fprintf(stderr, "UDP: bad checksum\n");
		goto drop;
	}

	return;
drop:
	mb_free(m);
}
