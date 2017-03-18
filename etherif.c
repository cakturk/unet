#include <stdio.h>
#include "netsniff.h"
#include "netif.h"
#include "arp.h"
#include "mbuf.h"

static char fmt_buf[1024];
static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

void ip_input(struct netif *ifp, struct mbuf *m);

void
eth_input(struct netif *netif, struct mbuf *m)
{
	struct machdr *hdr;

	hdr = mb_htrim(m, sizeof(*hdr));
	sb_reset(&sb);
	eth_print(hdr, &sb);

	switch (hdr->type) {
	case ntohs(ETH_P_IP):
		ip_input(netif, m);
		break;
	case ntohs(ETH_P_ARP):
		arp_recv(netif, m);
		break;
	default:
		mb_chain_free(m);
		break;
	}
}

void
eth_output(struct netif *ifp, struct mbuf *m,
	   uint8_t *dst, uint16_t ethproto)
{
	struct machdr *eh;

	eh = mb_push(m, sizeof(*eh));
	memcpy(eh->dst, dst, ETH_ALEN);
	memcpy(eh->src, &ifp->hwaddr, ETH_ALEN);
	eh->type = htons(ethproto);

	netif_xmit(ifp, m);
}
