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

static char fmt_buf[1024];

static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

static struct netif netif;

int main(int argc, char *argv[])
{
	char iface_name[IFNAMSIZ];
	char pbuf[4096];
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

	while (1) {
		struct machdr *mac;
		int len;

		len = read(netif.tunfd, pbuf, sizeof(pbuf));
		printf("read: %d bytes\n", len);

		mac = mac_hdr(pbuf);
		sb_reset(&sb);
		eth_print(mac, &sb);
		printf("machdr: %s\n", sb.buf);
	}

	exit(EXIT_SUCCESS);
}
