#include <stddef.h>	/* NULL */
#include "mbuf.h"
#include "netsniff.h"

static void
ip_print(void)
{
}

void
ip_input(struct netif *ifp, struct mbuf *m)
{
	struct iphdr *iph;

	iph = mb_head(m);
	mb_htrim(m, ip_hdrlen(iph));
}
