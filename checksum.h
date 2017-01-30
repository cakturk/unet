#ifndef CHECKSUM_H_
#define CHECKSUM_H_

uint16_t ip_csum(const void *d, size_t len, uint32_t sum);
uint16_t ip_udp_csum(uint32_t src, uint32_t dst, struct udphdr *uh);

#endif
