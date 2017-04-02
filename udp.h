#ifndef UDP_H_
#define UDP_H_
struct mbuf;
struct netif;

void udp_init(void);
void udp_input(struct netif *ifp, struct mbuf *m);
void udp_output(struct netif *ifp,
		ipv4_t saddr, uint16_t sport,
		ipv4_t daddr, uint16_t dport,
		const char *buf, size_t len);

#endif
