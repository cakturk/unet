#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  /* open */
#include <unistd.h> /* read */
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../checksum.c"

ssize_t read_nr_bytes(int fd, void *buf, size_t n)
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
}

int
checksum_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
