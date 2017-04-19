#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "netif.h"

struct shell_struct;

#define BITS_PER_INT 32
#define GENMASKINT(h, l) \
	(((~0U) << (l)) & (~0U >> (BITS_PER_INT - 1 - (h))))
#define GEN_NETMASK(nr) \
	nr == 0 ? nr : GENMASKINT(31, 32 - nr)

static void show_ip(FILE *fp, const struct netif *ifp)
{
	char addr[INET_ADDRSTRLEN];
	char mask[INET_ADDRSTRLEN];

	fprintf(fp, "ip addr %s netmask %s\n",
		inet_ntop(AF_INET, &ifp->ipaddr, addr, INET_ADDRSTRLEN),
		inet_ntop(AF_INET, &ifp->mask, mask, INET_ADDRSTRLEN));
}

static void set_iface_address(struct netif *ifp, const char *addr)
{

	union {
		uint8_t  d[sizeof(uint32_t)];
		uint32_t ip;
	} u;
	uint32_t mask_bitnr;
	int rc;
	char c;

	rc = sscanf(addr, "%hhu.%hhu.%hhu.%hhu/%u%c",
		    &u.d[0], &u.d[1], &u.d[2], &u.d[3],
		    &mask_bitnr, &c);
	if (rc != 5) {
		fprintf(stderr, "Bad IP/netmask format: %s\n", addr);
		return;
	}
	if (mask_bitnr > 32) {
		fprintf(stderr, "netmask must be in the range of 0-32\n");
		return;
	}
	ifp->mask.addr = htonl(GEN_NETMASK(mask_bitnr));
	ifp->ipaddr.addr = u.ip;
}

static void usage(void)
{
	fprintf(stderr,
		"set [address]/[netmask] - set IP address and netmask\n"
		"get  - display IP address\n"
		"help - display this message\n");
}

void ip_cmd_main(struct shell_struct *s, int argc, char *const argv[])
{
	const char *argv1 = argv[1];
	struct netif *ifp;

	ifp = netif_default_iface();

	if (argc == 1 || (argc == 2 && !strcmp(argv1, "get")))
		return show_ip(stdout, ifp);

	if (argc == 3 && !strcmp(argv1, "set"))
		return set_iface_address(ifp, argv[2]);

	if (argc == 2 && !strcmp(argv1, "help"))
		return usage();

	fprintf(stderr, "Command failed, try \"ip help\".\n");
	usage();
}
