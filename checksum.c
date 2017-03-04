#include <stddef.h>	/* size_t */
#include <stdint.h>
#include <netinet/in.h>	/* IPPROTO_* */

#include "netsniff.h"
#include "mbuf.h"

/*
 * XXX: This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */
uint16_t ip_csum(const void *d, size_t len, uint32_t sum)
{
	const uint8_t *buf = d;
	uint32_t acc = sum;
	uint16_t *word;

	while (len > 1) {
		word = (uint16_t *)buf;
		acc += *word;
		len -= 2;
		buf += 2;
	}

	if (len)
		acc += *buf;

	acc = (acc & 0xffff) + (acc >> 16);
	acc = (acc & 0xffff) + (acc >> 16);

	return ~(acc);
}

/*
 * XXX - This checksum code has the following limitation.
 * All the mbufs in the chain must contain an even number
 * of bytes with the exception of last mbuf. So bear that
 * in mind when modifying the code.
 */
uint16_t ip_csum_mb(const struct mbuf *m, uint32_t sum)
{
	uint32_t acc;
	uint8_t *buff;
	const uint16_t *word;
	unsigned int len;
	int byte_swapped = 0;
	union {
		uint16_t s[2];
		uint32_t l;
	} l_util;

	do {
		buff = mb_head(m);
		len = mb_datalen(m);
		acc = 0;

		while (len > 1) {
			word = (uint16_t *)buff;
			acc += *word;
			len -= 2;
			buff += 2;
		}

		if (len) {
			int b;

			b = byte_swapped;
			byte_swapped = !byte_swapped;
			acc += *buff;
			if (b)
				goto swap_bytes;
			else
				goto do_sum;
		}

		if (byte_swapped) {
swap_bytes:
			l_util.l = acc;
			l_util.s[0] = __const_swab16(l_util.s[0]);
			l_util.s[1] = __const_swab16(l_util.s[1]);
			acc = l_util.l;
		}
do_sum:
		sum += acc;
		m = m->m_next;
	} while (m);

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);

	return ~(sum);
}

uint16_t rfc1071_csum(const void *d, size_t count, uint32_t sum)
{
	const uint8_t *addr = d;

	while (count > 1) {
		sum += *(uint16_t *)addr;
		addr += 2;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0)
		sum += *(uint8_t *)addr;

	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~(sum);
}

uint16_t ip_udp_csum(uint32_t src, uint32_t dst, struct udphdr *uh)
{
	uint32_t sum = 0x0000;

	sum += src;
	sum += dst;

	sum += IPPROTO_UDP << 8;
	sum += uh->len;

	return ip_csum(uh, ntohs(uh->len), sum);
}

uint16_t ip_udp_csum_mb(uint32_t src, uint32_t dst, const struct mbuf *m)
{
	struct udphdr *uh;
	uint32_t sum = 0x0000;

	uh = mb_head(m);

	sum += src;
	sum += dst;

	sum += IPPROTO_UDP << 8;
	sum += uh->len;

	return ip_csum_mb(m, sum);
	/* return ip_csum(uh, ntohs(uh->len), sum); */
}
