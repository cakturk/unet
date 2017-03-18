#include <stdio.h>
#include "minunit.h"
#include "../mbuf.c"

#define MB_POOL_LASTELEM (MBUF_POOL_LEN - 1)

static void test_setup()
{
	mb_pool_init();
}

static int chain_nr_elems(struct mbuf *m)
{
	int count = 0;

	while (m) {
		m = m->m_next;
		count++;
	}

	return count;
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

/*
 * Test for edge cases like calling the func with a nrbuffs of zero
 */
MU_TEST(test_mb_pool_chain_alloc_with_a_size_of_zero)
{
	struct mbuf *m;

	m = mb_pool_chain_alloc(0);
	mu_check(m == NULL);
	mu_check(free_list != NULL);
}

struct mbuf_chain_test_info {
	struct {
		struct mbuf *m;

		/* number of elements in the chain */
		int n;
	} d[MBUF_POOL_LEN];
	struct mbuf **free_list;
	int tot_free;
};

static void assert_alloc(int n, struct mbuf_chain_test_info *ti, int idx)
{
	struct mbuf *m;
	int nr_mbuffs;

	m = mb_pool_chain_alloc(n);
	nr_mbuffs = chain_nr_elems(m);

	/* Check if we really allocated a chain of 'n' mbufs */
	mu_check(m != NULL);
	mu_assert_int_eq(n, nr_mbuffs);

	ti->d[idx].m = m;
	ti->d[idx].n = n;
	ti->tot_free -= n;

	mu_assert_int_eq(ti->tot_free, chain_nr_elems(*ti->free_list));
}

static void assert_free(struct mbuf_chain_test_info *ti, int idx)
{
	int nr_mbuffs;

	mb_pool_chain_free(ti->d[idx].m);
	ti->tot_free += ti->d[idx].n;

	nr_mbuffs = chain_nr_elems(*ti->free_list);
	mu_assert_int_eq(ti->tot_free, nr_mbuffs);
}

MU_TEST(test_mb_pool_chain_alloc)
{
	struct mbuf_chain_test_info info = {
		.free_list = &free_list,
		.tot_free  = MBUF_POOL_LEN
	};
	int i = 0;

	mu_assert_int_eq(info.tot_free, chain_nr_elems(free_list));

	assert_alloc(3, &info, i++);
	assert_alloc(4, &info, i++);
	assert_alloc(1, &info, i++);
	assert_alloc(5, &info, i++);
	assert_alloc(3, &info, i++);

	while (--i >= 0)
		assert_free(&info, i);
}

MU_TEST(test_mb_alloc_mtu)
{
	struct mbuf *m;
	size_t total_len = 0;

	m = mb_alloc_mtu();
	mu_check(m != NULL);

	for (; m; m = m->m_next)
		total_len += MLEN;

	mu_check(total_len >= (MTU_SIZE + MB_IP_ALIGN));
}

MU_TEST(test_mb_pool_sg_alloc)
{
	struct mbuf_iovec miov;
	struct mbuf *m;
	size_t total_len = 0;
	int err;

	err = mb_pool_sg_alloc(&miov);
	mu_assert_int_eq(7, err);

	for (m = miov.buffs[0]; m; m = m->m_next)
		total_len += mb_datalen(m);
	mu_assert_int_eq(7, chain_nr_elems(miov.buffs[0]));
	mu_assert_int_eq(MLEN*7 - MB_IP_ALIGN, total_len);

	err = mb_pool_sg_alloc(&miov);
	mu_assert_int_eq(7, err);

	total_len = 0;
	for (m = miov.buffs[0]; m; m = m->m_next)
		total_len += mb_datalen(m);
	mu_assert_int_eq(7, chain_nr_elems(miov.buffs[0]));
	mu_assert_int_eq(MLEN*7 - MB_IP_ALIGN, total_len);

	err = mb_pool_sg_alloc(&miov);
	mu_assert_int_eq(2, err);

	total_len = 0;
	for (m = miov.buffs[0]; m; m = m->m_next)
		total_len += mb_datalen(m);
	mu_assert_int_eq(2, chain_nr_elems(miov.buffs[0]));
	mu_assert_int_eq(MLEN*2 - MB_IP_ALIGN, total_len);
}

MU_TEST(test_mb_pool_sg_free_excess)
{
	struct mbuf_iovec mi;
	int rc;

	rc = mb_pool_sg_alloc(&mi);
	mu_assert_int_eq(7, rc);
	mu_assert_int_eq(7, chain_nr_elems(mi.buffs[0]));

	mb_pool_sg_free_excess(&mi, 463);
	mu_assert_int_eq(3, chain_nr_elems(mi.buffs[0]));
	mu_assert_int_eq(MBUF_POOL_LEN - 3, chain_nr_elems(free_list));
}

MU_TEST_SUITE(test_suite)
{
	MU_SUITE_CONFIGURE(test_setup, NULL);
	MU_RUN_TEST(test_mb_last);
	MU_RUN_TEST(test_mb_pool_alloc);
	MU_RUN_TEST(test_mb_pool_chain_alloc_with_a_size_of_zero);
	MU_RUN_TEST(test_mb_pool_chain_alloc);
	MU_RUN_TEST(test_mb_alloc_mtu);
	MU_RUN_TEST(test_mb_pool_sg_alloc);
	MU_RUN_TEST(test_mb_pool_sg_free_excess);
}

int
mbuf_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
