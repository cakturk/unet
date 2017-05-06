#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  /* open */
#include <unistd.h> /* read */
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "minunit.h"
#include "../checksum.c"

static ssize_t read_nr_bytes(int fd, void *buf, size_t n)
{
	char *p = buf;
	ssize_t total = 0;

	for (;;) {
		ssize_t ret;
		if (n < 1)
			break;
		ret = read(fd, buf, n);
		if (ret < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		if (ret == 0)
			break;
		total += ret;
		p += ret;
		n -= ret;
	}

	return total;
}

static ssize_t read_random_nr_bytes(void *buf, size_t n)
{
	int fd;
	ssize_t tot;

	if ((fd = open("/dev/urandom", O_RDONLY)) == -1)
		return -1;
	tot = read_nr_bytes(fd, buf, n);
	close(fd);
	return tot;
}

struct chain_test_helper {
	struct mbuf   *head;
	unsigned char *chain_cont;
	unsigned int   size;
};

static void fill_mbuffs_with_random_bytes(struct chain_test_helper *mth,
					  int nr_mbuffs, ...)
{
	struct mbuf **m;
	unsigned char *cp;
	va_list ap;

	mth->size = 0;
	m = &mth->head;
	mth->chain_cont = cp = malloc(MLEN * nr_mbuffs);
	va_start(ap, nr_mbuffs);
	while (nr_mbuffs-- > 0) {
		int n = va_arg(ap, int);
		assert(n <= MLEN);
		*m = malloc(sizeof(**m));
		mb_init(*m);

		n = read_random_nr_bytes(cp, n);
		assert(n > 0);
		memcpy(mb_put(*m, n), cp, n);
		cp += n;
		mth->size += n;
		m = &(*m)->m_next;
	}
	va_end(ap);
}

MU_TEST(test_mbuf_chain)
{
	struct chain_test_helper cth;
	uint16_t rfcsum, sum;

	fill_mbuffs_with_random_bytes(&cth, 5, 1, 109, 5, 8, 3);
	rfcsum = ip_csum(cth.chain_cont, cth.size, 0);
	sum    = ip_csum_mb(cth.head, 0);
	mu_assert_int_eq(rfcsum, sum);

	fill_mbuffs_with_random_bytes(&cth, 6, 4, 50, 49, 10, 3, 7);
	rfcsum = ip_csum(cth.chain_cont, cth.size, 0);
	sum    = ip_csum_mb(cth.head, 0);
	mu_assert_int_eq(rfcsum, sum);

	fill_mbuffs_with_random_bytes(&cth, 2, 102, 102);
	rfcsum = ip_csum(cth.chain_cont, cth.size, 0);
	sum    = ip_csum_mb(cth.head, 0);
	mu_assert_int_eq(rfcsum, sum);

	fill_mbuffs_with_random_bytes(&cth, 1, 80);
	rfcsum = ip_csum(cth.chain_cont, cth.size, 0);
	sum    = ip_csum_mb(cth.head, 0);
	mu_assert_int_eq(rfcsum, sum);
}

MU_TEST(test_cksum_mb)
{
	struct mbuf m1, m2;
	unsigned char buff[2 * MLEN];
	unsigned char c1, c2;
	ssize_t n;
	size_t pkt_sz = 4;
	uint16_t rfcsum, sum, mbsum;

	mb_init(&m1);
	mb_init(&m2);
	m1.m_next = &m2;

	n = read_random_nr_bytes(buff, pkt_sz);
	mu_check(n == pkt_sz);

	/* first half */
	memcpy(mb_put(&m1, 2), &buff[0], 2);
	mu_assert_int_eq(2, mb_datalen(&m1));
	/* second half */
	memcpy(mb_put(&m2, 1), &buff[2], 1);
	mu_assert_int_eq(1, mb_datalen(&m2));

	c1 = buff[2];
	c2 = ((uint8_t *)mb_head(&m2))[0];
	mu_check(c1 == c2);
	mu_assert_int_eq(c1, c2);

	rfcsum = rfc1071_csum(buff, n-1, 0);
	sum = ip_csum(buff, n-1, 0);
	mbsum = ip_csum_mb(&m1, 0);
	mu_assert_int_eq(rfcsum, sum);
	mu_assert_int_eq(rfcsum, mbsum);
}

MU_TEST(test_rfc1071_sum)
{
	int fd;
	size_t n, pkt_sz = 20;
	uint16_t rfcsum, sum;
	unsigned char buf[512];

	if ((fd = open("/dev/urandom", O_RDONLY)) == -1)
		mu_fail("/dev/urandom could not be opened!");
again:
	n = read_nr_bytes(fd, buf, pkt_sz++);
	mu_check(n != 0);

	rfcsum = rfc1071_csum(buf, n, 0);
	sum = ip_csum(buf, n, 0);
	mu_assert_int_eq(rfcsum, sum);
	if (pkt_sz <= 255)
		goto again;

	close(fd);
}

MU_TEST_SUITE(test_suite)
{
	MU_RUN_TEST(test_rfc1071_sum);
	MU_RUN_TEST(test_mbuf_chain);
	MU_RUN_TEST(test_cksum_mb);
}

int
checksum_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
