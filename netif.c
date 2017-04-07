#include <unistd.h>       /* close() syscall */
#include <errno.h>
#include <fcntl.h>        /* open() syscall */
#include <arpa/inet.h>    /* inet_pton */
#include <sys/ioctl.h>    /* ioctl SIOCGIFHWADDR */
#include <sys/uio.h>      /* scatter/gather IO */
#include <net/if.h>       /* struct ifreq */
#include <string.h>       /* memcpy and so on */
#include <stdio.h>
#include <stdlib.h>       /* exit */
#include <linux/if_tun.h>

#include "netif.h"
#include "etherif.h"
#include "mbuf.h"
#include "udp.h"

static struct netif *default_iface;

static void
netif_set_default_iface(struct netif *ifp)
{
	default_iface = ifp;
}

struct netif *netif_default_iface(void)
{
	return default_iface;
}

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

static int __attribute__ ((unused))
hwaddr_get(int fd, void *dst)
{
	struct ifreq ifr;

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
		return -1;

	memcpy(dst, ifr.ifr_hwaddr.sa_data, HWADDR_LEN);
	return 0;
}

int
netif_init(struct netif *netif, char *ifnam, const char *ipaddr,
	   const hwaddr_t *ether)
{
	int tunfd;

	if ((tunfd = tun_alloc(ifnam)) <= 0)
		return -1;
	netif->tunfd = tunfd;

	if (!inet_pton(AF_INET, ipaddr, &netif->ipaddr))
		return -1;
	memcpy(&netif->hwaddr, ether, ETH_ALEN);
	netif_set_default_iface(netif);
	return 0;
}

int
netif_poll(struct netif *netif)
{
	struct mbuf_iovec mi;
	ssize_t rc;

	mb_pool_init();
	udp_init();

	for (;;) {
		rc = mb_pool_sg_alloc(&mi);
		if (rc < 1) {
			fprintf(stderr, "Out of mem!\n");
			exit(EXIT_FAILURE);
		}
		rc = readv(netif->tunfd, mi.iov, ARRAY_SIZE(mi.iov));
		if (rc <= 0) {
			fprintf(stderr, "Error reading interface\n");
			exit(EXIT_FAILURE);
		}
		mb_pool_sg_free_excess(&mi, rc);
		eth_input(netif, mi.buffs[0]);
	}

	return 0;
}

void
netif_xmit(struct netif *ifp, struct mbuf *m)
{
	ssize_t n;
	unsigned int dlen;

	while ((dlen = mb_datalen(m))) {
		n = write(ifp->tunfd, mb_head(m), dlen);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		mb_htrim(m, n);
	}
	mb_pool_chain_free(m);
}
