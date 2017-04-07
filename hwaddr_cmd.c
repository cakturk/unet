#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "netif.h"

static void display_hwaddr(FILE *fp, const struct netif *ifp)
{
	const uint8_t *addr = ifp->hwaddr.data;

	fprintf(fp, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static void set_iface_hwaddress(struct netif *ifp, const char *new)
{
	int rc;
	uint8_t *mac = ifp->hwaddr.data, tmp;

	rc = sscanf(new, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx%c",
		    &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], &tmp);
	if (rc != 6)
		fprintf(stderr, "Bad mac address format: %s\n", new);
}

static void usage(void)
{
	fprintf(stderr,
		"set xx:xx:xx:xx:xx - where xx is between 0 - 255\n"
		"get  - display mac address\n"
		"help - display this message\n");
}

void hwaddr_main(int argc, char *const *argv)
{
	const char *argv1 = argv[1];
	struct netif *ifp;

	ifp = netif_default_iface();

	if (argc == 1 || (argc == 2 && !strcmp(argv1, "get")))
		return display_hwaddr(stdout, ifp);

	if (argc == 3 && !strcmp(argv1, "set"))
		return set_iface_hwaddress(ifp, argv[2]);

	if (argc == 2 && !strcmp(argv1, "help"))
		return usage();

	fprintf(stderr, "Command failed, try \"hwaddr help\".\n");
	usage();
}
