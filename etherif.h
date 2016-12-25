#ifndef _ETHERIF_H
#define _ETHERIF_H

struct netif;
struct mbuf;

void eth_input(struct netif *netif, struct mbuf *m);
void eth_output(struct netif *ifp, struct mbuf *m, uint8_t *dst);

#endif /* end of include guard: _ETHERIF_H */
