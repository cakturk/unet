/*
 * main.c
 *
 * Main entry point of this program.
 *
 * Copyright (C) Cihangir Akturk, 2016
 */
#include <unistd.h>
#include <net/if.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "netsniff.h"
#include "netif.h"

static struct netif netif;

int main(int argc, char *argv[])
{
	char iface_name[IFNAMSIZ];
	uint8_t *mac;
	ipv4_t *ip;

	if (argc > 1)
		strcpy(iface_name, argv[1]);
	else
		strcpy(iface_name, "tap0");

	if (netif_init(&netif, iface_name, "10.10.120.40") == -1)
		exit(EXIT_FAILURE);

	mac = netif.hwaddr.data;
	ip = &netif.ipaddr;
	/* display mac address */
	printf("iface: %s, hwaddr: %02x:%02x:%02x:%02x:%02x:%02x, "
	       "ipaddr: %d.%d.%d.%d\n",
	       iface_name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
	       ip->data[0], ip->data[1], ip->data[2], ip->data[3]);

	netif_poll(&netif);

	exit(EXIT_SUCCESS);
}
