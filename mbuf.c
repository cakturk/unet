#include <stdlib.h>
#include "mbuf.h"

#ifndef MBUF_POOL_LEN
#define MBUF_POOL_LEN 16
#endif

static struct mbuf mpool[MBUF_POOL_LEN];
static struct mbuf *free_list;

void mbuf_link(struct mbuf **head, struct mbuf *first, struct mbuf *last);
static inline struct mbuf *mb_pool_alloc(void);
static inline void mb_pool_free(struct mbuf *m);

struct mbuf *mb_alloc(void)
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

void mb_free(struct mbuf *m)
{
#if MBUF_POOL_LEN
	mb_pool_free(m);
#else
	free(m);
#endif
}

void mb_pool_init(void)
{
	int i;

	free_list = &mpool[0];
	for (i = 1; i < sizeof(mpool) / sizeof(mpool[0]); ++i)
		mpool[i - 1].m_next = &mpool[i];
	mpool[MBUF_POOL_LEN - 1].m_next = NULL;
}

static inline struct mbuf *mb_pool_alloc(void)
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

void mb_pool_chain_free(struct mbuf *m)
{
	mbuf_link(&free_list, m, mb_last(m));
}

static inline void mb_pool_free(struct mbuf *m)
{
	m->m_next = free_list;
	free_list = m;
}

void mbuf_link(struct mbuf **head, struct mbuf *first, struct mbuf *last)
{
	last->m_next = *head;
	*head = first;
}

struct mbuf *mbuf_unlink(struct mbuf **first, struct mbuf **last)
{
	struct mbuf *m = *first;
	*first = (*last)->m_next;
	*last = NULL;
	return m;
}
