#ifndef ARP_H_
#define ARP_H_
/*
 * Address Resolution Protocol.
 * See RFC 826 for protocol description.
 * - https://tools.ietf.org/html/rfc826
 */
struct arphdr {
	uint16_t ar_hrd;	/* hardware type, one of: */
#define ARPHRD_ETHER	1	/* ethernet */
#define ARPHRD_FRELAY	15	/* frame relay */

	uint16_t ar_pro;	/* protocol type */
	uint8_t	 ar_hln;	/* length of hardware address */
	uint8_t	 ar_pln;	/* length of protocol address */
	uint16_t ar_op;		/* arp opcode, one of: */
#define	ARPOP_REQUEST	 1	/* arp request */
#define	ARPOP_REPLY	 2	/* arp reply */
#define	ARPOP_REVREQUEST 3	/* rarp request */
#define	ARPOP_REVREPLY	 4	/* rarp reply */
#define ARPOP_INVREQUEST 8	/* InArp request */
#define ARPOP_INVREPLY	 9	/* InArp reply */
/*
 * The remaining fields are variable in size,
 * according to the sizes above.
 */
	uint8_t ar_pld[];	/* pointer to the start of following data */
#ifdef COMMENT_ONLY
	uint8_t	ar_sha[];	/* sender hardware address */
	uint8_t	ar_spa[];	/* sender protocol address */
	uint8_t	ar_tha[];	/* target hardware address */
	uint8_t	ar_tpa[];	/* target protocol address */
#endif
};

#define __arp_hdr_len(ar_hln, ar_pln)					\
	(sizeof(struct arphdr) + 2 * (ar_hln) + 2 * (ar_pln))
#define arp_hdr_len(ap) (__arp_hdr_len((ap)->ar_hln, (ap)->ar_pln))
#define ARP4_HDR_LEN	28

#define arp_hdr(p) ((struct arphdr *)(p))

#define ar_sha(p) ((p)->ar_pld)
#define ar_spa(p) ((p)->ar_pld + (p)->ar_hln)
#define ar_tha(p) ((p)->ar_pld + (p)->ar_hln + (p)->ar_pln)
#define ar_tpa(p) ((p)->ar_pld + 2 * (p)->ar_hln + (p)->ar_pln)

struct netif;
struct mbuf;

void arp_print(struct arphdr *hdr);
void arp_recv(struct netif *ifp, struct mbuf *m);
int arp_resolve(struct netif *ifp, ipv4_t dstaddr,
		struct mbuf *m, hwaddr_t *dsthw);

#endif /* end of include guard: ARP_H_ */
