#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "netif.h"

struct shell_struct;

static void display_route(FILE *fp, const struct netif *ifp)
{
	char addr[INET_ADDRSTRLEN];

	fprintf(fp, "%-20s %s\n%-20s %s\n",
		"Destination", "Gateway", "default",
		inet_ntop(AF_INET, &ifp->gateway, addr, INET_ADDRSTRLEN));
}

static void set_iface_gw(struct netif *ifp, const char *rtentry)
{
	if (inet_pton(AF_INET, rtentry, &ifp->gateway) != 1)
		fprintf(stderr, "Bad route format: %s\n", rtentry);
}

static void usage(void)
{
	fprintf(stderr,
		"setgw - set the default route\n"
		"get   - display the contents of routing tables\n"
		"help  - display this message\n");
}

void route_main(struct shell_struct *s, int argc, char *const argv[])
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
