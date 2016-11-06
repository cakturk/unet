#ifndef ARP_H_
#define ARP_H_
/*
 * Address Resolution Protocol.
 * See RFC 826 for protocol description.
 * - https://tools.ietf.org/html/rfc826
 */
struct arphdr {
	uint16_t ar_hrd;	/* hardware type, one of: */
#define ARPHRD_ETHER 	1	/* ethernet */
#define ARPHRD_FRELAY 	15	/* frame relay */

	uint16_t ar_pro;	/* protocol type */
	uint8_t	 ar_hln;	/* length of hardware address */
	uint8_t	 ar_pln;	/* length of protocol address */
	uint16_t ar_op;		/* arp opcode, one of: */
#define	ARPOP_REQUEST	 1	/* arp request */
#define	ARPOP_REPLY	 2	/* arp reply */
#define	ARPOP_REVREQUEST 3	/* rarp request */
#define	ARPOP_REVREPLY	 4	/* rarp reply */
#define ARPOP_INVREQUEST 8 	/* InArp request */
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

#endif /* end of include guard: ARP_H_ */
