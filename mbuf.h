#ifndef MBUF_H_
#define MBUF_H_

#include <stdint.h>

#ifndef MSIZE
#define MSIZE 256
#endif
#define MLEN (MSIZE - sizeof(struct m_hdr))

struct mbuf;

/* Header present at the beginning of every mbuf */
struct m_hdr {
	struct mbuf *mh_next;
	uint8_t     *mh_head;
	uint8_t     *mh_tail;
};

struct mbuf {
	struct m_hdr m_hdr;
	uint8_t      m_data[MLEN];
};

#define m_next m_hdr.mh_next
#define m_head m_hdr.mh_head
#define m_tail m_hdr.mh_tail

#define MB_IP_ALIGN 2

/*
 * Following ascii diagram illustrates the layout of mbuf data structure.
 *
 *           +--+-------------+
 *   m_hdr   |  |             |
 *           |  |             |
 *           +--------------------> &m_data[0]
 *           |  |             |
 *           |  |             |
 *           |  |             +---> m_head
 *           |  |             |
 *           +  |             |
 * m_data[MLEN] |             |
 *           +  |             |
 *           |  |             |
 *           |  |             |
 *           |  |             +---> m_tail
 *           |  |             |
 *           +--+-------------+---> &m_data[MLEN]
 */

struct mbuf *mb_alloc(void);
void mb_free(struct mbuf *m);

static inline void mb_init(struct mbuf *m)
{
	m->m_next = NULL;
	m->m_head = m->m_data;
	m->m_tail = m->m_data;
}

/*
 * Increase the headroom of an empty mbuf by reducing the tail room.
 * This is only allowed for an empty buffer. This function roughly
 * does the same thing as skb_reserve of Linux kernel.
 */
static inline void mb_reserve(struct mbuf *m, unsigned int len)
{
	m->m_head += len;
	m->m_tail += len;
}

static inline unsigned int mb_headroom(struct mbuf *m)
{
	return m->m_head - m->m_data;
}

static inline unsigned int mb_tailroom(struct mbuf *m)
{
	return &m->m_data[MLEN] - m->m_tail;
}

static inline void *mb_push(struct mbuf *m, unsigned int len)
{
	m->m_head -= len;
	return m->m_head;
}

static inline void *mb_put(struct mbuf *m, unsigned int len)
{
	uint8_t *oldtail = m->m_tail;
	m->m_tail += len;
	return oldtail;
}

static inline void mb_htrim(struct mbuf *m, unsigned int len)
{
	m->m_head -= len;
}

#endif /* end of include guard: MBUF_H_ */
