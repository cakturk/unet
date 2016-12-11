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
	uint8_t *head = m->m_head;

	hdr = mac_hdr(head);
	sb_reset(&sb);
	eth_print(hdr, &sb);
	//printf("machdr: %s\n", sb.buf);

	switch (hdr->type) {
	case ntohs(ETH_P_IP):
		//printf("IP datagram received\n");
		break;
	case ntohs(ETH_P_ARP):
		arp_print(arp_hdr(head + sizeof(*hdr)));
		arp_recv(netif, head + sizeof(*hdr));
		break;
	default:
		//printf("eth type: %s\n", ethertype_to_str(ntohs(hdr->type)));
		break;
	}
}
