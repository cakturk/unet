#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <arpa/inet.h>
#include "netsniff.h"

#define CAPTURE_INF -1

/* MAC + IP + TCP hdr */
#define MIN_SNAPLEN 54

#define err_exit(msg) do {		\
	fprintf(stderr, "%s", msg);	\
	exit(EXIT_FAILURE);		\
} while (0)

static void
pkt_handler_en10mb(u_char *usr, const struct pcap_pkthdr *pkt,
		   const u_char *d);
static void
pkt_handler_linux_sll(u_char *usr, const struct pcap_pkthdr *pkt,
		      const u_char *d);
const char *
mac_str(char *__restrict buf, uint8_t *__restrict addr);

int main(int argc, char *argv[])
{
	static struct program_options opts;
	struct bpf_program fp;
	char errbuff[PCAP_ERRBUF_SIZE];
	pcap_t *pcap;
	int err, linktype;

	if (get_program_options(argc, argv, &opts))
		err_exit("could not get program options\n");

	if (!opts.interface)
		opts.interface = "any";
	if (opts.snaplen < MIN_SNAPLEN)
		opts.snaplen = MIN_SNAPLEN;

	pcap = pcap_open_live(opts.interface, opts.snaplen,
			      opts.promisc, 2000, errbuff);
	if (!pcap)
		err_exit("cannot open network interface\n");

	err = pcap_compile(pcap, &fp, opts.bpf_expr, 0, -1);
	if (err)
		err_exit(pcap_geterr(pcap));

	err = pcap_setfilter(pcap, &fp);
	if (err)
		err_exit(pcap_geterr(pcap));

	linktype = pcap_datalink(pcap);
	printf("datalink: %s\n", pcap_datalink_val_to_name(linktype));

	switch (linktype) {
	case DLT_EN10MB:
		err = pcap_loop(pcap, CAPTURE_INF,
				pkt_handler_en10mb, NULL);
		break;
	case DLT_LINUX_SLL:
		err = pcap_loop(pcap, CAPTURE_INF,
				pkt_handler_linux_sll, NULL);
		break;
	default:
		err_exit("Datalink type is not yet supported!\n");
	}

	switch (err) {
	case 0:
		printf("netsniff terminated successfully\n");
		break;
	case -1:
	case -2:
		printf("netsniff terminated with error: %d\n", err);
		break;
	}

	exit(EXIT_SUCCESS);
}

static char fmt_buf[1024];

static struct strbuf sb = {
	.size = sizeof(fmt_buf),
	.buf = fmt_buf,
};

static void etherframe_print(u_char *usr, const struct pcap_pkthdr *pkt,
			     const u_char *d, uint16_t ethtype)
{
	struct iphdr *ip;
	struct tcphdr *th;
	struct udphdr *uh;
	uint16_t ethtype_le;

	ethtype_le = ntohs(ethtype);

	switch (ethtype_le) {
	case ETH_P_IP:
		ip = ip_hdr(d);
		sb_append_str(&sb, "IP: ");
		iphdr_print(ip, &sb);

		switch (ip->protocol) {
		case IPPROTO_TCP:
			th = tcp_hdr(d + ip_hdrlen(ip));
			sb_append_str(&sb, "; TCP: ");
			tcp_print(th, &sb);
			break;

		case IPPROTO_UDP:
			uh = udp_hdr(d + ip_hdrlen(ip));
			sb_append_str(&sb, "; UDP: ");
			udp_print(uh, &sb);
			break;

		default:
			sb_append_char(&sb, ' ');
			sb_append_str(&sb, ipproto_str(ip->protocol));
		}

		break;
	default:
		/* FIXME: This code is open to buffer overrun errors */
		sb_append_str(&sb, "ether type: ");
		sb.len += sprintf(sb_curr(&sb), "0x%04x ", ethtype_le);
		sb_append_str(&sb, ethertype_to_str(ethtype_le));
	}
}

static void
pkt_handler_en10mb(u_char *usr, const struct pcap_pkthdr *pkt,
		   const u_char *d)
{
	struct machdr *mac = mac_hdr(d);

	sb_reset(&sb);
	eth_print(mac, &sb);
	sb_append_str(&sb, "; ");
	etherframe_print(usr, pkt, d + ETH_HLEN, mac->type);
	fprintf(stdout, "pkt: %s\n", sb.buf);
}

static void
pkt_handler_linux_sll(u_char *usr, const struct pcap_pkthdr *pkt,
		      const u_char *d)
{
	struct dlt_linux_sll *ll = dlt_linux_sll_hdr(d);

	BUILD_BUG_ON(sizeof(*ll) != 16);

	sb_reset(&sb);
	etherframe_print(usr, pkt, ll->payload, ll->proto_type);
	fprintf(stdout, "pkt: %s\n", sb.buf);
}
