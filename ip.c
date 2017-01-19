#include <stdio.h>
#include <stddef.h>	/* NULL */
#include "mbuf.h"
#include "netif.h"
#include "netsniff.h"

static char fmt_buf[1024];

static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf
};

uint32_t ip_csum(const void *buf, size_t len, uint32_t sum);
uint16_t ip_udp_csum(uint32_t src, uint32_t dst, struct udphdr *uh);

void
ip_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr *iph;
	struct udphdr *uh;
	uint16_t cksum;

	iph = mb_head(m);
	mb_htrim(m, ip_hdrlen(iph));

	cksum = ip_csum(iph, ip_hdrlen(iph), 0x0000);
	sb_reset(&sb);
	if (iphdr_print(iph, &sb) == 0)
		printf("ip: %s, computed: %#x\n", sb.buf, cksum);

	switch (iph->protocol) {
	case IPPROTO_UDP:
		uh = mb_head(m);
		printf("uhhhh: %u\n", uh->csum);
		if (uh->csum) {
			printf("csum: %#x, comp: %#x\n", uh->csum,
			       ip_udp_csum(iph->saddr, iph->daddr, uh));
		}
		break;
	case IPPROTO_TCP:
		printf("TCP\n");
		break;
	}
}
