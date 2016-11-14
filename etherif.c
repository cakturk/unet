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
	struct arphdr *a;
	struct machdr *hdr;
	char sip[sizeof("255.255.255.255")];
	char tip[sizeof(sip)];

	hdr = mac_hdr(buf);
	sb_reset(&sb);
	eth_print(hdr, &sb);
	//printf("machdr: %s\n", sb.buf);

	switch (hdr->type) {
	case ntohs(ETH_P_IP):
		//printf("IP datagram received\n");
		break;
	case ntohs(ETH_P_ARP):
		a = arp_hdr(buf + sizeof(*hdr));
		printf("ARP packet\n"
		       "ar_hrd: %hu\n"
		       "ar_pro: %hu\n"
		       "ar_hln: %u\n"
		       "ar_pln: %u\n"
		       "ar_op:  %hu\n"
		       "ar_spa: %s\n"
		       "ar_tpa: %s\n\n",
		       ntohs(a->ar_hrd),
		       ntohs(a->ar_pro),
		       a->ar_hln,
		       a->ar_pln,
		       ntohs(a->ar_op),
		       inet_ntop(AF_INET, ar_spa(a), sip, 32),
		       inet_ntop(AF_INET, ar_tpa(a), tip, 32));
		break;
	default:
		//printf("eth type: %s\n", ethertype_to_str(ntohs(hdr->type)));
		break;
	}
}
