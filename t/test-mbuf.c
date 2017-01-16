#include <stdio.h>
#include "minunit.h"
#include "../mbuf.c"

#define MB_POOL_LASTELEM (MBUF_POOL_LEN - 1)

static void test_setup()
{
	mb_pool_init();
}

MU_TEST(test_mb_last)
{
	struct mbuf *m, *last, **pp;

	m = &mpool[0];
	last = mb_last(m);

	mu_check(last == &mpool[MB_POOL_LASTELEM]);
	mu_check(last->m_next == NULL);

	pp = mb_lastpp(&m);
	mu_check(pp == &mpool[MB_POOL_LASTELEM].m_next);
	mu_check(*pp == NULL);
}

MU_TEST(test_mb_pool_alloc)
{
	struct mbuf *m, *n, *last;
	struct mbuf *allocd[MBUF_POOL_LEN];
	int i;

	for (i = 0; i < MBUF_POOL_LEN; ++i) {
		m = mb_pool_alloc();
		n = &mpool[i];
		allocd[i] = m;

		mu_check(m != NULL);
		mu_check(m == n);
		mu_check(m->m_next == NULL);
	}

	/* Out-of-mem */
	m = mb_pool_alloc();
	mu_check(m == NULL);

	/* free_list should be NULL */
	mu_check(free_list == NULL);

	/* Now we release the allocated mbufs */
	for (i = 0; i < MBUF_POOL_LEN; ++i) {
		mb_pool_free(allocd[i]);
		mu_check(free_list == allocd[i]);
	}

	mu_check(free_list == &mpool[MB_POOL_LASTELEM]);

	last = mb_last(free_list);
	mu_check(last == &mpool[0]);
	mu_check(last->m_next == NULL);

	/*
	 * Ouch! lots of duplicated code. Anyway it's no big deal
	 * considering we are just testing.
	 */
	for (i = MB_POOL_LASTELEM; i >= 0; --i) {
		m = mb_pool_alloc();
		n = &mpool[i];
		allocd[i] = m;

		mu_check(m != NULL);
		mu_check(m == n);
		mu_check(m->m_next == NULL);
	}
}

MU_TEST_SUITE(test_suite)
{
	MU_SUITE_CONFIGURE(test_setup, NULL);
	MU_RUN_TEST(test_mb_last);
	MU_RUN_TEST(test_mb_pool_alloc);
}

int
mbuf_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
