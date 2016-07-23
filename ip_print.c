#include <errno.h>
#include <stdio.h>
#include "netsniff.h"

#if 0
enum {
	IPPROTO_IP = 0,               /* Dummy protocol for TCP               */
	IPPROTO_ICMP = 1,             /* Internet Control Message Protocol    */
	IPPROTO_IGMP = 2,             /* Internet Group Management Protocol   */
	IPPROTO_IPIP = 4,             /* IPIP tunnels (older KA9Q tunnels use 94) */
	IPPROTO_TCP = 6,              /* Transmission Control Protocol        */
	IPPROTO_EGP = 8,              /* Exterior Gateway Protocol            */
	IPPROTO_PUP = 12,             /* PUP protocol                         */
	IPPROTO_UDP = 17,             /* User Datagram Protocol               */
	IPPROTO_IDP = 22,             /* XNS IDP protocol                     */
	IPPROTO_TP = 29,              /* SO Transport Protocol Class 4        */
	IPPROTO_DCCP = 33,            /* Datagram Congestion Control Protocol */
	IPPROTO_IPV6 = 41,            /* IPv6-in-IPv4 tunnelling              */
	IPPROTO_RSVP = 46,            /* RSVP Protocol                        */
	IPPROTO_GRE = 47,             /* Cisco GRE tunnels (rfc 1701,1702)    */
	IPPROTO_ESP = 50,             /* Encapsulation Security Payload protocol */
	IPPROTO_AH = 51,              /* Authentication Header protocol       */
	IPPROTO_MTP = 92,             /* Multicast Transport Protocol         */
	IPPROTO_BEETPH = 94,          /* IP option pseudo header for BEET     */
	IPPROTO_ENCAP = 98,           /* Encapsulation Header                 */
};
#endif
const char *ipproto_str(int proto)
{
	switch (proto) {
	case IPPROTO_IP:   return "Dummy TCP";
	case IPPROTO_ICMP: return "ICMP";
	case IPPROTO_IGMP: return "IGMP";
	case IPPROTO_IPIP: return "IPIP";
	case IPPROTO_TCP:  return "TCP";
	case IPPROTO_UDP:  return "UDP";
	default:
			   return "Uknown ipproto";
	}
}

int iphdr_print(struct iphdr *iph, struct strbuf *sb)
{
	const char *s;
	int len;
	char ihl[16];
	char flags[10];

	if (iph->version == 6) {
		const char msg[] = "IPv6 headers are not yet supported";
		size_t msglen = sizeof(msg);
		if (sb_room(sb) < msglen)
			return -ENOBUFS;;
		sb_append_str(sb, msg);
		return 0;
	}

	len = sprintf(ihl, "hdrlen: %d ", iph->ihl * 4);
        len += sprintf(flags, "[DF%dMF%d] ", ip_df(iph), ip_mf(iph));

	if (sb_room(sb) < (INET_ADDRSTRLEN * 2) + 3 + len)
		return -ENOBUFS;

	sb_append_str(sb, ihl);
	sb->len += sprintf(sb_curr(sb), "tot: %u ", ntohs(iph->tot_len));
	sb_append_str(sb, flags);

	s = inet_ntop(AF_INET, &iph->saddr, sb_curr(sb), INET_ADDRSTRLEN);
	if (!s)
		return -errno;
	sb->len += strlen(s);
	sb_append_str(sb, "->");
	s = inet_ntop(AF_INET, &iph->daddr, sb_curr(sb), INET_ADDRSTRLEN);
	if (!s)
		return -errno;
	sb->len += strlen(s);

	return 0;
}
