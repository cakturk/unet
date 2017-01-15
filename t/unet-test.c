int arp_test(void);

int
main(int argc, char *argv[])
{
	int retval = 0;

	retval |= arp_test();
	return retval;
}
