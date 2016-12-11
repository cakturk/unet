#include <stdio.h>
#include "netsniff.h"
#include "arp.h"
#include "mbuf.h"

struct netif;

static char fmt_buf[1024];
static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

void
eth_input(struct netif *netif, struct mbuf *m)
{
	struct machdr *hdr;

	hdr = mb_htrim(m, sizeof(*hdr));
	sb_reset(&sb);
	eth_print(hdr, &sb);

	switch (hdr->type) {
	case ntohs(ETH_P_IP):
		//printf("IP datagram received\n");
		break;
	case ntohs(ETH_P_ARP):
		arp_recv(netif, m);
		break;
	default:
		break;
	}
}
