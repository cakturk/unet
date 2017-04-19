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
#include <poll.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "netsniff.h"
#include "netif.h"
#include "shell.h"

static struct netif netif;

#define MACADDR ((hwaddr_t *)"\x56\x85\x6f\x7f\xa0\xc1")

int main(int argc, char *argv[])
{
	struct pollfd fds[2];
	struct shell_struct *sh;
	char iface_name[IFNAMSIZ];
	uint8_t *mac;
	ipv4_t *ip;

	if (argc > 1)
		strcpy(iface_name, argv[1]);
	else
		strcpy(iface_name, "tap0");

	if (netif_init(&netif, iface_name, "172.28.128.44", MACADDR) == -1)
		exit(EXIT_FAILURE);

	mac = netif.hwaddr.data;
	ip = &netif.ipaddr;
	/* display mac address */
	printf("iface: %s, hwaddr: %02x:%02x:%02x:%02x:%02x:%02x, "
	       "ipaddr: %d.%d.%d.%d\n",
	       iface_name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
	       ip->data[0], ip->data[1], ip->data[2], ip->data[3]);

	fds[0].fd = netif.tunfd;
	fds[0].events = POLLIN;

	/* netif_poll(&netif); */
	sh = shell_init(stdin, "unet-shell-> ");

	fds[1].fd = fileno(sh->fp);
	fds[1].events = POLLIN;

	for (;;) {
		int rc;

		do
			rc = poll(fds, 2, -1);
		while (rc == -1 && errno == EINTR);

		if (rc == 0) {
			fprintf(stderr, "No events found\n");
			break;
		}

		if (0 && fds[0].revents & POLLIN) {
			/* printf("pkt received\n"); */
		}
		if (fds[1].revents & POLLIN) {
			int ok;

			/* printf("input received\n"); */
			ok = sh->process_input(sh);
			if (!ok)
				break;
		}
	}

	exit(EXIT_SUCCESS);
}
