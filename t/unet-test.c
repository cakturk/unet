int arp_test(void);
int mbuf_test(void);
int checksum_test(void);

int
main(int argc, char *argv[])
{
	int retval = 0;

	retval |= arp_test();
	retval |= mbuf_test();
	retval |= checksum_test();
	return retval;
}
