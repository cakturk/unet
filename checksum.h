#ifndef CHECKSUM_H_
#define CHECKSUM_H_
#include <stdint.h>

struct udphdr;
struct mbuf;

uint32_t ip_csum_nocompl(const void *d, size_t len, uint32_t sum);
static inline uint16_t csum_fold(uint32_t sum)
{
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return (uint16_t)sum;
}
static inline uint16_t ip_csum(const void *d, size_t len, uint32_t sum)
{
	return (uint16_t)~csum_fold(ip_csum_nocompl(d, len, sum));
}
uint16_t ip_udp_csum(uint32_t src, uint32_t dst, struct udphdr *uh);
uint16_t ip_udp_csum_mb(uint32_t src, uint32_t dst, const struct mbuf *m);

#endif
