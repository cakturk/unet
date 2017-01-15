#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../arp.c"

static void test_setup()
{
	memset(arp_tab, 0, sizeof(arp_tab));
}

MU_TEST(test_arp_newent)
{
	ipv4_t addr = { .data = {192, 168, 1, 11} };
	struct arpentry *ent = arp_newentry(addr);

	mu_check(ent->ae_st == ARP_E_INCOMPLETE);
	mu_check(ent->ae_ip.addr == addr.addr);
}

MU_TEST(test_arp_lookup)
{
	ipv4_t addr = { .data = {192, 168, 1, 11} };
	struct arpentry *ent, *ret;

	ret = arp_lookup(addr);
	mu_check(ret == NULL);

	ent = arp_newentry(addr);
	ret = arp_lookup(addr);
	mu_check(ent == ret);
	mu_check(ent->ae_ip.addr == addr.addr);
}

MU_TEST_SUITE(test_suite)
{
	MU_SUITE_CONFIGURE(test_setup, NULL);
	MU_RUN_TEST(test_arp_newent);
	MU_RUN_TEST(test_arp_lookup);
}

int
arp_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
