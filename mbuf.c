#include <stdlib.h>
#include "mbuf.h"

struct mbuf *mb_alloc(void)
{
	struct mbuf *m;

	m = malloc(sizeof(struct mbuf));
	m->m_next = NULL;
	m->m_head = m->m_data;
	m->m_head = m->m_data;

	return m;
}

void mb_free(struct mbuf *m)
{
	free(m);
}
