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

MU_TEST(test_arp_resolve)
{
	ipv4_t addr = { .data = {192, 168, 1, 41} };
	struct mbuf m1, m2, m3;
	struct arpentry *ent;
	hwaddr_t ether;
	int err;

	err = __arp_resolve(addr, NULL, &ether);
	mu_assert_int_eq(0, err);

	ent = &arp_tab[0];
	mu_check(ent->ae_wq_head == NULL);
	mu_assert_int_eq(ARP_E_INCOMPLETE, ent->ae_st);

	mb_init(&m1);

	err = __arp_resolve(addr, &m1, &ether);
	mu_assert_int_eq(0, err);
	mu_check(ent->ae_wq_head == &m1);
	mu_check(ent->ae_wq_head->m_next == NULL);

	mb_init(&m2);
	m1.m_next = &m2;

	mb_init(&m3);
	err = __arp_resolve(addr, &m3, &ether);
	mu_assert_int_eq(0, err);
	mu_check(ent->ae_wq_head == &m1);
	mu_check(ent->ae_wq_head->m_next == &m2);
	mu_check(ent->ae_wq_head->m_next->m_next == &m3);
	mu_check(ent->ae_wq_head->m_next->m_next->m_next == NULL);
}

MU_TEST_SUITE(test_suite)
{
	MU_SUITE_CONFIGURE(test_setup, NULL);
	MU_RUN_TEST(test_arp_newent);
	MU_RUN_TEST(test_arp_lookup);
	MU_RUN_TEST(test_arp_resolve);
}

int
arp_test(void)
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return minunit_fail;
}
