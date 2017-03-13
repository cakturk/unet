#include <sys/uio.h>
#include "mbuf.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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

int mb_pool_alloc_vectored(struct mbuf_iovec *miov)
{
	struct mbuf *m = free_list;
	unsigned int n = 0;

	if (!m)
		return -1;

	mbuf_map_iov(&miov->iov[n], m);
	mbuf_set_data_ptrs(m, MB_IP_ALIGN, MLEN);
	n++;

	for (; n < ARRAY_SIZE(miov->iov) && m->m_next; n++) {
		m = m->m_next;
		mbuf_map_iov(&miov->iov[n], m);
		mbuf_set_data_ptrs(m, 0, MLEN);
	}
	miov->list_head = mbuf_unlink(&free_list, &m->m_next);
	miov->list_tail = m;

	return n;
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
