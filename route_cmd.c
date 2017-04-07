#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "netif.h"

#define BITS_PER_INT 32
#define GENMASKINT(h, l) \
	(((~0U) << (l)) & (~0U >> (BITS_PER_INT - 1 - (h))))

#define GEN_NETMASK(nr) \
	nr == 0 ? nr : GENMASKINT(31, 32 - nr)

static void display_route(FILE *fp, const struct netif *ifp)
{
	char addr[INET_ADDRSTRLEN];

	fprintf(fp, "%-20s %s\n%-20s %s\n",
		"Destination", "Gateway", "default",
		inet_ntop(AF_INET, &ifp->gateway, addr, INET_ADDRSTRLEN));
}

static void set_iface_gw(struct netif *ifp, const char *rtentry)
{
	union {
		uint32_t ip;
		uint8_t  d[sizeof(uint32_t)];
	} u;
	uint32_t mask_bitnr;
	int rc;
	char c;

	rc = sscanf(rtentry, "%hhu.%hhu.%hhu.%hhu/%u%c",
		    &u.d[0], &u.d[1], &u.d[2], &u.d[3],
		    &mask_bitnr, &c);
	if (rc != 5) {
		fprintf(stderr, "Bad route format: %s\n", rtentry);
		return;
	}
	if (mask_bitnr > 32) {
		fprintf(stderr, "netmask must be in the range of 0-32\n");
		return;
	}
	ifp->mask.addr = htonl(GEN_NETMASK(mask_bitnr));
	ifp->gateway.addr = u.ip;
}

static void usage(void)
{
	fprintf(stderr,
		"setgw - set the default route\n"
		"get   - display the contents of routing tables\n"
		"help  - display this message\n");
}

void route_main(int argc, char *const argv[])
{
	const char *argv1 = argv[1];
	struct netif *ifp;

	ifp = netif_default_iface();

	if (argc == 1 || (argc == 2 && !strcmp(argv1, "get")))
		return display_route(stdout, ifp);

	if (argc == 3 && !strcmp(argv1, "setgw"))
		return set_iface_gw(ifp, argv[2]);

	if (argc == 2 && !strcmp(argv1, "help"))
		return usage();

	fprintf(stderr, "Command failed, try \"route help\".\n");
	usage();
}

int ankara(int argc, char *argv[])
{
	char buf[INET_ADDRSTRLEN];
	int rc;
	const char *rtentry = "6.10.20.30/24";
	union {
		uint32_t ip;
		uint8_t  d[sizeof (uint32_t)];
	} u;
	uint32_t mask;
	char tmp;

	rc = sscanf(rtentry, "%hhu.%hhu.%hhu.%hhu/%u%c",
		    &u.d[0], &u.d[1], &u.d[2], &u.d[3],
		    &mask, &tmp);

	printf("%u.%u.%u.%u/%d %d\n", u.d[0], u.d[1], u.d[2], u.d[3], mask, rc);

	/* mask = htonl(GENMASKINT(32, 4)); */
	mask = htonl(GEN_NETMASK(26));

	/* printf("%#lx\n", GENMASK(31, 1)); */
	printf("%#x\n", ntohl(mask));
	printf("%s\n", inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN));

	return 0;
}
