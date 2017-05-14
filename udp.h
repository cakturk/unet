#ifndef UDP_H_
#define UDP_H_

#include "netif.h"

struct mbuf;
struct netif;
struct udphdr;
struct iphdr;

void udp_init(void);
uint16_t udp_next_port(void);
void udp_input(struct netif *ifp, struct mbuf *m);
void udp_output(struct netif *ifp,
		ipv4_t saddr, uint16_t sport,
		ipv4_t daddr, uint16_t dport,
		const char *buf, size_t len);

void udp_bind(uint16_t port, void (*on_udp_dgram)(const struct netif *ifp,
						  const struct iphdr *ih,
						  const struct udphdr *uh));
void udp_unbind(void);
#endif
