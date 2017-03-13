#include <sys/time.h>
#include <stdio.h>
#include "netsniff.h"
#include "mbuf.h"
#include "checksum.h"
#include "netif.h"

struct netif;

static uint16_t udp_id;

void
udp_init(void)
{
	struct timeval tv;
	int err;

	err = gettimeofday(&tv, NULL);
	udp_id = err ? 0xcafe : (tv.tv_sec + tv.tv_usec) & 0xffff;
}

static unsigned count;
void
udp_output(struct netif *ifp);

void
udp_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr  *iph;
	struct udphdr *uh;

	iph = mb_head(m);
	mb_htrim(m, ip_hdrlen(iph));

	if (mb_datalen(m) < sizeof(*uh)) {
		fprintf(stderr, "UDP: packet too small for the header\n");
		goto drop;
	}

	uh = mb_head(m);
	if (ntohs(uh->len) < mb_datalen(m)) {
		fprintf(stderr, "UDP: packet too small for the payload\n");
		goto drop;
	}

	if (uh->csum && ip_udp_csum_mb(iph->saddr, iph->daddr, m) != 0) {
		fprintf(stderr, "UDP: bad checksum\n");
		goto drop;
	}
	printf("receiving UDP\n");
	if (count++ > 3) {
		printf("sending UDP\n");
		udp_output(ifp);
	}
	return;
drop:
	mb_free(m);
}

void
ip_output(struct netif *ifp, struct mbuf *m,
	  uint16_t id, uint8_t proto,
	  uint32_t saddr, uint32_t daddr);

void
udp_output(struct netif *ifp)
{
	struct mbuf	*m;
	struct udphdr	*uh;
	ipv4_t saddr = { .data = {172, 28, 128, 44} };
	ipv4_t daddr = { .data = {172, 28, 128, 5} };
	const char msg[] = "Beam me up, Scotty!\n";

	if ((m = mb_alloc()) == NULL)
		return;

	/* Reserve space for layer 2 headers */
	mb_reserve(m, ETH_HLEN + sizeof(struct iphdr));
	/* UDP header */
	uh = mb_put(m, sizeof(*uh));
	/* UDP payload */
	strcpy(mb_put(m, sizeof(msg)-1), msg);

	uh->sport = htons(12345);
	uh->dport = htons(33333);
	uh->len = htons(sizeof(*uh) + sizeof(msg)-1);
	uh->csum = 0;

	if ((uh->csum = ip_udp_csum_mb(saddr.addr, daddr.addr, m)) == 0)
		uh->csum = ~0;

	ip_output(ifp, m, udp_id++, IPPROTO_UDP, saddr.addr, daddr.addr);
}