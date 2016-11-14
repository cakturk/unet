#include <unistd.h>       /* close() syscall */
#include <fcntl.h>        /* open() syscall */
#include <arpa/inet.h>    /* inet_pton */
#include <sys/ioctl.h>    /* ioctl SIOCGIFHWADDR */
#include <net/if.h>       /* struct ifreq */
#include <string.h>       /* memcpy and so on */
#include <stdio.h>
#include <linux/if_tun.h>

#include "netif.h"

/*
 * Taken from Kernel Documentation/networking/tuntap.txt
 */
static int tun_alloc(char *dev)
{
	struct ifreq ifr;
	int fd, err;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
		fprintf(stderr, "Cannot open TUN/TAP dev\n"
			"Make sure one exists with "
			"'$ mknod /dev/net/tap c 10 200'\n");
		return -1;
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
		perror("ERR: Could not ioctl tun");
		close(fd);
		return err;
	}

	strcpy(dev, ifr.ifr_name);
	return fd;
}

static int
hwaddr_get(int fd, void *dst)
{
	struct ifreq ifr;

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
		return -1;

	memcpy(dst, ifr.ifr_hwaddr.sa_data, HWADDR_LEN);
	return 0;
}

int
netif_init(struct netif *netif, char *ifnam, const char *ipaddr)
{
	int tunfd;

	if ((tunfd = tun_alloc(ifnam)) <= 0)
		return -1;
	netif->tunfd = tunfd;

	if (!inet_pton(AF_INET, ipaddr, &netif->ipaddr))
		return -1;
	if (hwaddr_get(tunfd, &netif->hwaddr))
		return -1;
	return 0;
}

#define INB_LEN 2048
static char inbuf[INB_LEN];
void eth_input(struct netif *netif, const void *buf);

int
netif_poll(struct netif *netif)
{
	ssize_t err;

	for (;;) {
		err = read(netif->tunfd, inbuf, INB_LEN);
		//printf("read: %zd bytes\n", err);
                (void)err;
		eth_input(netif, inbuf);
	}

	return 0;
}
