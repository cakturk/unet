#include <stdio.h>
#include <stddef.h>	/* NULL */
#include "netif.h"
#include "mbuf.h"
#include "udp.h"
#include "netsniff.h"
#include "checksum.h"
#include "etherif.h"
#include "arp.h"

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
		goto drop;
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
		/* FALLTHROUGH */
	default:
		goto drop;
	}
	return;
drop:
	mb_chain_free(m);
}

static inline ipv4_t route_dst(const struct netif *ifp, uint32_t daddr)
{
	if ((daddr & ifp->mask.addr) == (ifp->ipaddr.addr & ifp->mask.addr))
		return (ipv4_t)daddr;
	return ifp->gateway;
}

void
ip_output(struct netif *ifp, struct mbuf *m,
	  uint16_t id, uint8_t proto,
	  uint32_t saddr, uint32_t daddr)
{
	struct iphdr *ih;
	hwaddr_t      dsteth;
	ipv4_t	      dstip;

	ih = mb_push(m, sizeof(*ih));
	ih->version = 4;
	ih->ihl = 5;
	ih->tos	= 0;
	ih->tot_len = htons(mb_datalen(m));
	ih->id = id;
	ih->frag_off = 0;
	ih->ttl	= 30;
	ih->protocol = proto;
	ih->saddr = saddr;
	ih->daddr = daddr;

	/* Fill checksum field */
	ih->check = 0;
	ih->check = ip_csum(ih, ip_hdrlen(ih), 0);

	dstip = route_dst(ifp, daddr);
	if (arp_resolve(ifp, dstip, m, &dsteth))
		eth_output(ifp, m, dsteth.data, ETH_P_IP);
}
