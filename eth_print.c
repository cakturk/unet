#include <errno.h>
#include <stdio.h>
#include "netsniff.h"

const char *mac_str(char *__restrict buf, const uint8_t *__restrict addr)
{
	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return buf;
}

int eth_print(const struct machdr *mh, struct strbuf *sb)
{
	if (sb_room(sb) < (ETH_ADDRSTRLEN * 2) + 1)
		return -ENOBUFS;

	mac_str(sb_curr(sb), mh->src);
	sb->len += ETH_ADDRSTRLEN - 1;
	sb_append_char(sb, '>');
	mac_str(sb_curr(sb), mh->dst);
	sb->len += ETH_ADDRSTRLEN - 1;

	return 0;
}

const char *ethertype_to_str(uint16_t type)
{
	switch (type) {
	case ETH_P_IP:   return "IP";
	case ETH_P_IPV6: return "IPv6";
	case ETH_P_ARP:  return "ARP";
	case ETH_P_RARP: return "RARP";
	case ETH_P_DEC:  return "DEC";
	default:         return "Unknow ether type";
	}
}

int eth_print_type(uint16_t type, struct strbuf *sb)
{
	const char *typestr = ethertype_to_str(type);
	size_t len = strlen(typestr);

	if (sb_room(sb) < len)
		return -ENOBUFS;

	sb_append_str(sb, typestr);
	return 0;
}
