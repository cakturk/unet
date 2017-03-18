#include <sys/uio.h>
#include <assert.h>
#include "mbuf.h"

static struct mbuf mpool[MBUF_POOL_LEN];
static struct mbuf *free_list;

static inline void mbuf_link(struct mbuf **head, struct mbuf *first,
			     struct mbuf *last);
static inline struct mbuf *mbuf_unlink(struct mbuf **first, struct mbuf **last);

void mb_pool_init(void)
{
	int i;

	free_list = &mpool[0];
	for (i = 1; i < sizeof(mpool) / sizeof(mpool[0]); ++i)
		mpool[i - 1].m_next = &mpool[i];
	mpool[MBUF_POOL_LEN - 1].m_next = NULL;
}

struct mbuf *mb_pool_alloc(void)
{
	struct mbuf *m = NULL;

	if (free_list) {
		m = free_list;
		free_list = m->m_next;
		mb_init(m);
	}

	return m;
}

struct mbuf *mb_pool_chain_alloc(unsigned int nrbufs)
{
	struct mbuf *m, **curr = &free_list;

	if (!*curr || nrbufs < 1)
		return NULL;

	while (nrbufs--) {
		(*curr)->m_head = (*curr)->m_data;
		(*curr)->m_tail = (*curr)->m_data;
		curr = &(*curr)->m_next;

		if (!*curr)
			break;
	}

	m = free_list;
	free_list = *curr;
	*curr = NULL;

	return m;
}

static inline void
mbuf_map_iov(struct iovec *iov, const struct mbuf *m)
{
	iov->iov_base = m->m_head;
	iov->iov_len = mb_datalen(m);
}

static inline void
mbuf_set_data_ptrs(struct mbuf *m, size_t headoffset, size_t tailoffset)
{
	m->m_head = &m->m_data[headoffset];
	m->m_tail = &m->m_data[tailoffset];
}

static inline void
mbuf_set_datalen(struct mbuf *m, unsigned int len)
{
	m->m_tail = m->m_head + len;
}

int mb_pool_sg_alloc(struct mbuf_iovec *miov)
{
	struct mbuf *m = free_list;
	unsigned int n = 0;

	if (!m)
		return -1;

	mbuf_set_data_ptrs(m, MB_IP_ALIGN, MLEN);
	mbuf_map_iov(&miov->iov[n], m);
	miov->buffs[n++] = m;

	for (; n < ARRAY_SIZE(miov->iov) && m->m_next; n++) {
		m = m->m_next;
		mbuf_set_data_ptrs(m, 0, MLEN);
		mbuf_map_iov(&miov->iov[n], m);
		miov->buffs[n] = m;
	}
	mbuf_unlink(&free_list, &m->m_next);

	return n;
}

/**
 * Frees @len bytes worth of mbufs from the end of an mbuf chain
 * pointed to by @m
 */
void mb_pool_sg_free_excess(struct mbuf_iovec *mi, unsigned int bytes_in_use)
{
	struct mbuf *m;

	assert(bytes_in_use <= MTU_SIZE);
	m = mi->buffs[0];
	if (bytes_in_use > (MLEN - MB_IP_ALIGN)) {
		unsigned int nth;

		/* proceed to the next mbuf */
		m++;
		bytes_in_use -= MLEN - MB_IP_ALIGN;

		nth = bytes_in_use / MLEN;
		bytes_in_use -= nth * MLEN;
		m += nth;
	}

	mbuf_set_datalen(m, bytes_in_use);
	if (m->m_next) {
		mb_pool_chain_free(m->m_next);
		m->m_next = NULL;
	}
}

void mb_pool_chain_free(struct mbuf *m)
{
	mbuf_link(&free_list, m, mb_last(m));
}

void mb_pool_free(struct mbuf *m)
{
	m->m_next = free_list;
	free_list = m;
}

static inline void
mbuf_link(struct mbuf **head, struct mbuf *first, struct mbuf *last)
{
	last->m_next = *head;
	*head = first;
}

static inline struct mbuf *
mbuf_unlink(struct mbuf **first, struct mbuf **last)
{
	struct mbuf *m = *first;
	*first = *last;
	*last = NULL;
	return m;
}
