#include <errno.h>
#include <stdio.h>
#include "netsniff.h"

const char *mac_str(char *__restrict buf, const uint8_t *__restrict addr)
{
	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return buf;
}

const char *
macstr(void *__restrict str, const void *__restrict addr)
{
	int err;
	const uint8_t *m = addr;

	if (!str)
		str = malloc(ETH_ADDRSTRLEN);

	err = snprintf(str, ETH_ADDRSTRLEN, "%02x:%02x:%02x:%02x:%02x:%02x",
		       m[0], m[1], m[2], m[3], m[4], m[5]);

	return err >= ETH_ADDRSTRLEN ? NULL : str;
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
	default:         return "Unknown ether type";
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
