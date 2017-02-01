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
	mb_free(m);
}

void
ip_output(struct netif *ifp, struct mbuf *m,
	  uint16_t id, uint8_t proto,
	  uint32_t saddr, uint32_t daddr)
{
	struct iphdr *ih;
	hwaddr_t      desteth;
	ipv4_t	      ipv = { .addr = daddr };
	/* static uint8_t dest[] = {0xfe, 0x54, 0x00, 0x04, 0xc7, 0xf8}; */
	/* static uint8_t dest[] = {0x52, 0x54, 0x00, 0xb6, 0xe0, 0xc1}; */

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

	if (arp_resolve(ifp, ipv, m, &desteth))
		eth_output(ifp, m, desteth.data, ETH_P_IP);
}
