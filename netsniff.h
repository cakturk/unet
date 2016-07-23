/*
 * Copyright (C) 2015 Cihangir Akturk
 */
#ifndef _NETSNIF_H_
#define _NETSNIF_H_

#if defined(__APPLE__)
#define __BYTE_ORDER __BYTE_ORDER__
#define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define __BIG_ENDIAN __ORDER_BIG_ENDIAN__
#else
/* Linux specific header. Fix this include for other UNIX systems */
#include <endian.h>
#endif

#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define __LITTLE_ENDIAN_BITFIELD 1
#else
#  define __BIG_ENDIAN_BITFIELD 1
#endif

#define BPF_SZ 80

struct program_options {
	const char *interface;
	int	    snaplen;
	int	    count;
	int	    promisc;
	char	    bpf_expr[BPF_SZ];
};

struct strbuf {
	size_t  size;
	size_t  len;
	char   *buf;
};
#define sb_curr(sb) ((sb)->buf + (sb)->len)

static inline size_t sb_room(struct strbuf *sb)
{
	return sb->size - sb->len;
}
static inline void sb_reset(struct strbuf *sb)
{
	sb->len = 0;
}
static inline void sb_append_char(struct strbuf *sb, char c)
{
	sb->buf[sb->len++] = c;
	sb->buf[sb->len] = '\0';
}
static inline void sb_append(struct strbuf *sb, const void *s, size_t len)
{
	memcpy(sb->buf + sb->len, s, len);
	sb->len += len;
	sb->buf[sb->len] = '\0';
}
static inline void sb_append_str(struct strbuf *sb, const char *s)
{
	sb_append(sb, s, strlen(s));
}

/**
 * BUILD_BUG_ON - break compile if a condition is true.
 * Copied from linux kernel source tree
 */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/**
 * I could simply 'memcpy' the 'proto_type' field but I added
 * this structure definition in the hope that it will be useful
 * for the readers to help understand.
 */
struct dlt_linux_sll {
	uint16_t ptype;
	uint16_t arphrd_type;
	uint16_t ll_addr_len;
	uint8_t  ll_addr[8];
	uint16_t proto_type;
	uint8_t  payload[];
};

#define dlt_linux_sll_hdr(ptr) ((struct dlt_linux_sll *)(ptr))

#define ETH_ALEN 6        /* Octets in one ethernet addr   */
#define ETH_HLEN 14       /* Total octets in header.       */
#define ETH_ADDRSTRLEN 18 /* Total octets in header.       */

/* Ether protocols (type) */
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#define ETH_P_ARP       0x0806          /* Address Resolution packet    */
#define ETH_P_DEC       0x6000          /* DEC Assigned proto           */
#define ETH_P_RARP      0x8035          /* Reverse Addr Res packet      */
#define ETH_P_IPV6      0x86DD          /* IPv6 over bluebook           */

struct machdr {
	uint8_t	 dst[ETH_ALEN];
	uint8_t	 src[ETH_ALEN];
	uint16_t type;
};
#define mac_hdr(ptr) ((struct machdr *)(ptr))

#define IP_RE           0x8000          /* Flag: "Reserved"             */
#define IP_DF           0x4000          /* Flag: "Don't Fragment"       */
#define IP_MF           0x2000          /* Flag: "More Fragments"       */
#define IP_OFFSET       0x1FFF          /* "Fragment Offset" part       */

struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint8_t	   ihl:4,
		   version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	uint8_t    version:4,
		   ihl:4;
#else
#error  "Please fix endianness macros"
#endif
	uint8_t	   tos;
	uint16_t   tot_len;
	uint16_t   id;
	uint16_t   frag_off;
	uint8_t	   ttl;
	uint8_t	   protocol;
	uint16_t   check;
	uint32_t   saddr;
	uint32_t   daddr;
};
#define ip_hdr(ptr) ((struct iphdr *)(ptr))
static inline int ip_hdrlen(struct iphdr *iph)
{
	return iph->ihl * 4;
}

/* UDP data structure */
struct udphdr {
	uint16_t sport;
	uint16_t dport;
	uint16_t len;
	uint16_t csum;
};
#define udp_hdr(ptr) ((struct udphdr *)(ptr))

/* TCP segment */
struct tcphdr {
	uint16_t  source;
	uint16_t  dest;
	uint32_t  seq;
	uint32_t  ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint16_t  res1:4,
		  doff:4,
		  fin:1,
		  syn:1,
		  rst:1,
		  psh:1,
		  ack:1,
		  urg:1,
		  ece:1,
		  cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	uint16_t  doff:4,
		  res1:4,
		  cwr:1,
		  ece:1,
		  urg:1,
		  ack:1,
		  psh:1,
		  rst:1,
		  syn:1,
		  fin:1;
#else
#error  "Adjust your <endian.h> defines"
#endif
	uint16_t  window;
	uint16_t  check;
	uint16_t  urg_ptr;
};
#define tcp_hdr(ptr) ((struct tcphdr *)(ptr))

/* returns true if MF bit is set */
static inline int ip_mf(struct iphdr *iph)
{
	return !!(iph->frag_off & htons(IP_MF));
}
/* returns true if DF bit is set */
static inline int ip_df(struct iphdr *iph)
{
	return !!(iph->frag_off & htons(IP_DF));
}

extern int get_program_options(int argc, char **argv,
			       struct program_options *opts);
extern const char *ethertype_to_str(uint16_t type);
extern int eth_print(const struct machdr *mh, struct strbuf *sb);
extern int tcp_print(struct tcphdr *th, struct strbuf *sb);
extern int iphdr_print(struct iphdr *iph, struct strbuf *sb);
extern int udp_print(struct udphdr *uh, struct strbuf *sb);
extern const char *ipproto_str(int proto);

#endif /* end of include guard: _NETSNIF_H_ */
