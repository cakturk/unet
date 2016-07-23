#include <stdio.h>
#include "netsniff.h"

int tcp_print(struct tcphdr *th, struct strbuf *sb)
{
	char buf[128];

	sprintf(buf, "%u->%u, seq %u ack %u",
		ntohs(th->source), ntohs(th->dest),
		ntohs(th->seq), ntohs(th->ack_seq));
	sb_append_str(sb, buf);

	return 0;
}
