#include <stdio.h>
#include <errno.h>
#include "netsniff.h"

#define UDPSTR_MAX 24

int
udp_print(struct udphdr *uh, struct strbuf *sb)
{
	char tmp[UDPSTR_MAX];

	if (sb_room(sb) < UDPSTR_MAX)
		return -ENOBUFS;

	sprintf(tmp, "%d->%d len: %d", ntohs(uh->sport),
		ntohs(uh->dport), ntohs(uh->len));
	sb_append_str(sb, tmp);

	return 0;
}
