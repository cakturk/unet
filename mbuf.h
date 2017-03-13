#ifndef MBUF_H_
#define MBUF_H_

#ifndef MBUF_POOL_LEN
#define MBUF_POOL_LEN 16
#endif

#include <sys/uio.h>
#include <stddef.h>
#include <stdint.h>
#if !(MBUF_POOL_LEN)
#include <stdlib.h>
#endif

#define MTU_SIZE 1500
#define _howmany(x, y)  (((x) + ((y) - 1)) / (y))

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

struct mbuf_iovec {
	struct mbuf *list_head;
	struct mbuf *list_tail;
	struct iovec iov[_howmany(MTU_SIZE, MLEN)];
};

struct mbuf *mb_pool_alloc(void);
void mb_pool_free(struct mbuf *m);

struct mbuf *mb_pool_chain_alloc(unsigned int nrbufs);
void mb_pool_chain_free(struct mbuf *m);

static inline void mb_init(struct mbuf *m);

static inline struct mbuf *mb_alloc(void)
{
	struct mbuf *m;
#if MBUF_POOL_LEN
	m = mb_pool_alloc();
#else
	m = malloc(sizeof(struct mbuf));
	mb_init(m);
#endif
	return m;
}

static inline void mb_free(struct mbuf *m)
{
#if MBUF_POOL_LEN
	mb_pool_free(m);
#else
	free(m);
#endif
}

void mb_pool_init(void);

/*
 * Allocate @num_mbuffs number of mbufs. Returns a pointer to
 * the head of the mbuf chain.
 */
static inline struct mbuf *mb_alloc_chain(unsigned int num_mbuffs)
{
	return mb_pool_chain_alloc(num_mbuffs);
}

#define mb_alloc_mtu() mb_alloc_chain(_howmany(MTU_SIZE, MLEN))

int mb_pool_alloc_vectored(struct mbuf_iovec *miov);

/*
 * Free an entire mbuf chain pointed to by @m
 */
static inline void mb_chain_free(struct mbuf *m)
{
	mb_pool_chain_free(m);
}

static inline void mb_init(struct mbuf *m)
{
	m->m_next = NULL;
	m->m_head = m->m_data;
	m->m_tail = m->m_data;
}

static inline void *mb_head(const struct mbuf *m)
{
	return m->m_head;
}

static inline struct mbuf *mb_last(struct mbuf *m)
{
	while (m->m_next)
		m = m->m_next;
	return m;
}

static inline struct mbuf **mb_lastpp(struct mbuf **m)
{
	while (*m)
		m = &(*m)->m_next;
	return m;
}

static inline unsigned int mb_datalen(const struct mbuf *m)
{
	return m->m_tail - m->m_head;
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

static inline void *mb_htrim(struct mbuf *m, unsigned int len)
{
	uint8_t *oldhead = m->m_head;
	m->m_head += len;
	return oldhead;
}

#endif /* end of include guard: MBUF_H_ */
