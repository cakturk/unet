#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "netif.h"

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
	if (inet_pton(AF_INET, addr, &ifp->ipaddr) != 1)
		fprintf(stderr, "Bad IP address format: %s\n", addr);
}

static void usage(void)
{
	fprintf(stderr,
		"set XXX.XXX.XXX.XXX - where XXX is between 0 - 255\n"
		"get  - display IP address\n"
		"help - display this message\n");
}

void ip_cmd_main(int argc, char *const argv[])
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
