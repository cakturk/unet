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

static void
hwaddr_print(int fd)
{
	struct ifreq ifr;
	unsigned char *mac;
	int err;

	if ((err = ioctl(fd, SIOCGIFHWADDR, &ifr)) == -1) {
		printf("ERR: Could not get hwaddr: %s\n", strerror(errno));
		return;
	}

	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	/* display mac address */
	printf("hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n" ,
	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/*
 * Taken from Kernel Documentation/networking/tuntap.txt
 */
static int tun_alloc(char *dev)
{
	struct ifreq ifr;
	int fd, err;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
		printf("Cannot open TUN/TAP dev\n"
				"Make sure one exists with "
				"'$ mknod /dev/net/tap c 10 200'\n");
		exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));

	/* Flags: IFF_TUN   - TUN device (no Ethernet headers)
	 *        IFF_TAP   - TAP device
	 *
	 *        IFF_NO_PI - Do not provide packet information
	 */
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	if (*dev)
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);

	if ((err = ioctl(fd, TUNSETIFF, &ifr)) < 0 ){
		printf("ERR: Could not ioctl tun: %s\n", strerror(errno));
		close(fd);
		return err;
	}

	strcpy(dev, ifr.ifr_name);
	return fd;
}

static char fmt_buf[1024];

static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

int main(int argc, char *argv[])
{
	char iface_name[IFNAMSIZ];
	char pbuf[4096];
	int tun_fd;

	if (argc > 1)
		strcpy(iface_name, argv[1]);
	else
		strcpy(iface_name, "tap0");

	tun_fd = tun_alloc(iface_name);
	if (tun_fd <= 0)
		exit(EXIT_FAILURE);

	printf("iface: %s, tun_fd: %d, %zu\n", iface_name, tun_fd,
			sizeof(iface_name));
	hwaddr_print(tun_fd);

	while (1) {
		struct machdr *mac;
		int len;

		len = read(tun_fd, pbuf, sizeof(pbuf));
		printf("read: %d bytes\n", len);

		mac = mac_hdr(pbuf);
		sb_reset(&sb);
		eth_print(mac, &sb);
		printf("machdr: %s\n", sb.buf);
	}

	return 0;
}
