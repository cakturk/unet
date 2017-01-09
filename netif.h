#ifndef NETIF_H_
#define NETIF_H_

#include <stdint.h>

#define HWADDR_LEN 6

typedef struct {
	uint8_t data[HWADDR_LEN];
} hwaddr_t;

typedef struct {
	union {
		int32_t addr;
		uint8_t data[sizeof(int32_t)];
	};
} ipv4_t;

struct netif {
	int		tunfd;
	ipv4_t		ipaddr;
	hwaddr_t	hwaddr;
};

struct mbuf;

extern int netif_init(struct netif *netif, char *ifnam, const char *ipaddr,
		      const hwaddr_t *ether);
extern int netif_poll(struct netif *netif);
extern void netif_xmit(struct netif *ifp, struct mbuf *m);

#endif /* end of include guard: NETIF_H_ */
