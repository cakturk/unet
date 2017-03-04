#ifndef CHECKSUM_H_
#define CHECKSUM_H_

struct udphdr;
struct mbuf;

uint16_t ip_csum(const void *d, size_t len, uint32_t sum);
uint16_t ip_udp_csum(uint32_t src, uint32_t dst, struct udphdr *uh);
uint16_t ip_udp_csum_mb(uint32_t src, uint32_t dst, const struct mbuf *m);

#endif
