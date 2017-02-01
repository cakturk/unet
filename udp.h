#ifndef UDP_H_
#define UDP_H_
struct mbuf;
struct netif;

void udp_init(void);
void udp_input(struct netif *ifp, struct mbuf *m);

#endif
