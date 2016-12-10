#include "netsniff.h"
#include "arp.h"
#include <stdio.h>

struct netif;

static char fmt_buf[1024];
static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

void
eth_input(struct netif *netif, const void *buf)
{
	struct machdr *hdr;

	hdr = mac_hdr(buf);
	sb_reset(&sb);
	eth_print(hdr, &sb);
	//printf("machdr: %s\n", sb.buf);

	switch (hdr->type) {
	case ntohs(ETH_P_IP):
		//printf("IP datagram received\n");
		break;
	case ntohs(ETH_P_ARP):
		arp_print(arp_hdr(buf + sizeof(*hdr)));
		arp_recv(netif, buf + sizeof(*hdr));
		break;
	default:
		//printf("eth type: %s\n", ethertype_to_str(ntohs(hdr->type)));
		break;
	}
}
