#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "shell.h"
#include "netsniff.h"
#include "udp.h"
#include "unet.h"

enum {
	PROTO_UDP = 0,
	PROTO_TCP = 1
};

static int (*process_input_orig)(struct shell_struct *sh);
static uint16_t port = 0;
static uint32_t addr = 0;
static struct {
	const struct netif *ifp;
	uint32_t ipaddr;
	uint16_t port;
} peer_addr;

static void usage(void)
{
	fprintf(stderr,
		"nc [OPTIONS...] [hostname] [port]\n\n"
		"The options are as follows:\n\n"
		"-l listen for incoming connections\n"
		"-u use UDP instead of default TCP\n");
}

static bool
parse_port(const char *s, uint16_t *port)
{
	unsigned long u;
	char *endp;

	errno = 0;
	u = strtoul(s, &endp, 10);

	if (errno) {
		fprintf(stderr, "Failed to parse: %s\n", strerror(errno));
		return false;
	}
	if (*endp != '\0') {
		fprintf(stderr, "Extra characters on input block\n");
		return false;
	}
	if (u > 0xffff || u < 1) {
		fprintf(stderr, "Ports must be in the range of 1-65535\n");
		return false;
	}
	*port = htons(u);

	return true;
}

static bool
parse_inet_addr(const char *s, uint32_t *addr)
{
	int ok;

	ok = inet_pton(AF_INET, s, addr);
	if (!ok) {
		fprintf(stderr, "Bad IP format: %s\n", s);
		return false;
	}

	return true;
}

static void
nc_udp_recv(const struct netif *ifp, const struct iphdr *sih,
	    const struct udphdr *uh)
{
	pr_dbg("pkt recvd\n");
	write(STDOUT_FILENO, uh->payload, udp_payload_len(uh));
	peer_addr.ifp = ifp;
	peer_addr.ipaddr = sih->saddr;
	peer_addr.port = uh->sport;
}

static int nc_udp_send(struct shell_struct *sh)
{
	char buf[1024];

	if (!fgets(buf, sizeof(buf), sh->fp)) {
		udp_unbind();
		memset(&peer_addr, 0, sizeof(peer_addr));
		sh->process_input = process_input_orig;
		shell_display_prompt(sh);
		return 1;
	}

	if (!peer_addr.ifp || !peer_addr.ipaddr || !peer_addr.port)
		return 1;

	udp_output((struct netif *)peer_addr.ifp,
		   peer_addr.ifp->ipaddr, port,
		   (ipv4_t)peer_addr.ipaddr, peer_addr.port,
		   buf, strlen(buf));

	return 1;
}

void nc_main(struct shell_struct *s, int argc, char *const argv[])
{
	int proto, lflag, ch;

	if (!argv[1])
		goto out_usage;

	(void)proto;
	opterr = 0;
	proto = PROTO_TCP;
	lflag = 0;

	/* Reinitialize 'getopt'. Note that to re-initialize getopt on BSDs
	 * the variable 'optreset' must also be set to 1.
	 */
	optind = 1;
	while ((ch = getopt(argc, argv, "ul")) != -1) {
		switch (ch) {
		case 'u':
			proto = PROTO_UDP;
			break;
		case 'l':
			lflag = 1;
			break;
		case '?':
		default:
			goto out_usage;
		}
	}
	argc -= optind;
	argv += optind;

	if (!argv[0])
		goto out_usage;

	if (argv[0] && !argv[1]) { /* listening mode */
		if (!lflag)
			goto out_usage;
		if (!parse_port(argv[0], &port))
			return;
		printf("listening port: %s, %u\n", argv[0], port);
		udp_bind(port, nc_udp_recv);
		process_input_orig = s->process_input;
		s->process_input = nc_udp_send;
	} else if (argv[0] && argv[1] && !argv[2]) { /* client mode */
		if (lflag)
			goto out_usage;
		if (!parse_inet_addr(argv[0], &addr))
			return;
		if (!parse_port(argv[1], &port))
			return;
		printf("client: %s:%u\n", argv[0], port);
	}

	return;
out_usage:
	usage();
}
