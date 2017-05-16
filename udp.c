#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "netsniff.h"
#include "mbuf.h"
#include "checksum.h"
#include "netif.h"
#include "unet.h"

static uint16_t udp_id;
static uint16_t udp_port;
static struct {
	uint16_t port;
	void (*cb)(const struct netif *ifp, const struct iphdr *sih,
		   struct mbuf *m);
} port_waiter;

void
udp_init(void)
{
	struct timeval tv;
	int err;

	err = gettimeofday(&tv, NULL);
	udp_id = err ? 0xcafe : (tv.tv_sec + tv.tv_usec) & 0xffff;

	srand(tv.tv_sec);
	udp_port = (rand() % (0xffff - 1024)) + 1024;
}

uint16_t
udp_next_port(void)
{
	uint16_t ret;

	ret = udp_port++;
	udp_port = (udp_port % (0xffff - 1024)) + 1024;
	return ret;
}

/* static unsigned count; */
static void udp_test_output(struct netif *ifp);

void
udp_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr  *iph;
	struct udphdr *uh;

	iph = mb_head(m);
	mb_htrim(m, ip_hdrlen(iph));

	if (mb_datalen(m) < sizeof(*uh)) {
		fpr_dbg(stderr, "UDP: packet too small for the header\n");
		goto drop;
	}

	uh = mb_head(m);
	if (mb_datalen(m) < ntohs(uh->len)) {
		fpr_dbg(stderr, "UDP: packet too small for the payload\n");
		goto drop;
	}

	if (uh->csum && ip_udp_csum_mb(iph->saddr, iph->daddr, m) != 0) {
		fprintf(stderr, "UDP: bad checksum\n");
		goto drop;
	}
	pr_dbg("receiving UDP\n");
	if (!uh->dport || port_waiter.port != uh->dport)
		goto drop;
	port_waiter.cb(ifp, iph, m);
	return;
drop:
	mb_pool_chain_free(m);
}

void
ip_output(struct netif *ifp, struct mbuf *m,
	  uint16_t id, uint8_t proto,
	  uint32_t saddr, uint32_t daddr);

void
udp_output(struct netif *ifp,
	   ipv4_t saddr, uint16_t sport,
	   ipv4_t daddr, uint16_t dport,
	   const char *buf, size_t len)
{
	struct mbuf	*m;
	struct udphdr	*uh;

	if ((m = mb_alloc()) == NULL)
		return;

	/* Reserve space for layer 2 headers */
	mb_reserve(m, ETH_HLEN + sizeof(struct iphdr));
	/* UDP header */
	uh = mb_put(m, sizeof(*uh));
	/* UDP payload */
	memcpy(mb_put(m, len), buf, len);

	uh->sport = sport;
	uh->dport = dport;
	uh->len = htons(sizeof(*uh) + len);
	uh->csum = 0;

	if ((uh->csum = ip_udp_csum_mb(saddr.addr, daddr.addr, m)) == 0)
		uh->csum = ~0;

	ip_output(ifp, m, udp_id++, IPPROTO_UDP, saddr.addr, daddr.addr);
}

void udp_bind(uint16_t port, void (*on_udp_dgram)(const struct netif *ifp,
						  const struct iphdr *ih,
						  struct mbuf *m))
{
	port_waiter.port = port;
	port_waiter.cb = on_udp_dgram;
}

void udp_unbind(void)
{
	port_waiter.port = 0;
	port_waiter.cb = NULL;
}

static void __attribute__ ((unused))
udp_test_output(struct netif *ifp)
{
	ipv4_t saddr = { .data = {172, 28, 128, 44} };
	ipv4_t daddr = { .data = {172, 28, 128, 5} };
	const char msg[] = "Beam me up, Scotty!\n";

	udp_output(ifp, saddr, htons(12345), daddr, htons(33333), msg, sizeof(msg)-1);
}
