#include <stdio.h>
#include "netsniff.h"
#include "netif.h"
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

	printf("ARP packet\n"
	       "ar_hrd: %hu\n"
	       "ar_pro: %hu\n"
	       "ar_hln: %u\n"
	       "ar_pln: %u\n"
	       "ar_op:  %hu\n"
	       "ar_sha: %s\n"
	       "ar_spa: %s\n"
	       "ar_tha: %s\n"
	       "ar_tpa: %s\n\n",
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

void
arp_reply(struct netif *rcvif, struct arphdr *a)
{
	char tmp[4];

	a->ar_op  = ARPOP_REPLY;
	memswap(ar_tpa(a), ar_spa(a), tmp, 4);
	memcpy(ar_tha(a), ar_sha(a), a->ar_hln);
}

void
arp_recv(struct netif *rcvif, const char *buf)
{
	struct arphdr *hdr;

	hdr = arp_hdr(buf);
	switch (hdr->ar_op) {
	case ntohs(ARPOP_REQUEST):
		arp_print(hdr);
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
