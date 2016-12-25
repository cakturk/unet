#include <stddef.h>
#include <stdio.h>
#include "netsniff.h"
#include "netif.h"
#include "etherif.h"
#include "mbuf.h"
#include "arp.h"

static inline void memswap(void *__restrict dst, void *__restrict src,
			   void *__restrict tmp, size_t n);

void
arp_print(struct arphdr *hdr)
{
	char sip[sizeof("255.255.255.255")];
	char tip[sizeof(sip)];
	char sha[sizeof("ff:ff:ff:ff:ff:ff")];
	char tha[sizeof(sha)];

	printf("ARP packet (%zu) bytes\n"
	       "ar_hrd: %hu\n"
	       "ar_pro: %hu\n"
	       "ar_hln: %u\n"
	       "ar_pln: %u\n"
	       "ar_op:  %hu\n"
	       "ar_sha: %s\n"
	       "ar_spa: %s\n"
	       "ar_tha: %s\n"
	       "ar_tpa: %s\n\n",
	       arp_hdr_len(hdr),
	       ntohs(hdr->ar_hrd),
	       ntohs(hdr->ar_pro),
	       hdr->ar_hln,
	       hdr->ar_pln,
	       ntohs(hdr->ar_op),
	       macstr(sha, ar_sha(hdr)),
	       inet_ntop(AF_INET, ar_spa(hdr), sip, 32),
	       macstr(tha, ar_tha(hdr)),
	       inet_ntop(AF_INET, ar_tpa(hdr), tip, 32));
}

static void
arp_reply(struct netif *rcvif, struct arphdr *req)
{
	struct mbuf mbuf;
	struct arphdr *resp;

	mb_init(&mbuf);
	mb_reserve(&mbuf, ETH_HLEN);
	resp = mb_put(&mbuf, ARP4_HDR_LEN);
	memcpy(resp, req, offsetof(typeof(*resp), ar_op));
	resp->ar_op  = htons(ARPOP_REPLY);
	memcpy(ar_sha(resp), &rcvif->hwaddr, HWADDR_LEN);
	memcpy(ar_spa(resp), &rcvif->ipaddr, sizeof(rcvif->ipaddr));
	memcpy(ar_tha(resp), ar_sha(req), HWADDR_LEN);
	memcpy(ar_tpa(resp), ar_spa(req), sizeof(rcvif->ipaddr));
	printf("Response\n");
	arp_print(resp);
	eth_output(rcvif, &mbuf, ar_tha(resp));
}

void
arp_recv(struct netif *rcvif, struct mbuf *m)
{
	struct arphdr *hdr;

	hdr = arp_hdr(mb_htrim(m, sizeof(*hdr)));
	switch (hdr->ar_op) {
	case htons(ARPOP_REQUEST):
		arp_print(hdr);
		arp_reply(rcvif, hdr);
		break;
	default:
		break;
	}
}

static inline void
memswap(void *__restrict dst, void *__restrict src,
	void *__restrict tmp, size_t n)
{
	memcpy(tmp, dst, n);
	memcpy(dst, src, n);
	memcpy(src, tmp, n);
}
