#include <stdlib.h>
#include "mbuf.h"

struct mbuf *mb_alloc(void)
{
	struct mbuf *m;

	m = malloc(sizeof(struct mbuf));
	mb_init(m);

	return m;
}

void mb_free(struct mbuf *m)
{
	free(m);
}
