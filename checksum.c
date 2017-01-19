#include <stddef.h>	/* size_t */
#include <stdint.h>
#include <netinet/in.h>	/* IPPROTO_* */

#include "netsniff.h"

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
