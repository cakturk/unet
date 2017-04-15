#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct shell_cmd {
	const char *cmd_name;
	const char *cmd_help;
	void (*cmd_fn)(int argc, char *const argv[]);
};

extern void ip_cmd_main(int argc, char *const argv[]);
extern void hwaddr_main(int argc, char *const argv[]);
extern void route_main(int argc, char *const argv[]);
extern void nc_main(int argc, char *const argv[]);

static const struct shell_cmd cmd_list[] = {
	{
		.cmd_name = "ip",
		.cmd_help = "set/get interface IP address",
		.cmd_fn   = ip_cmd_main
	},
	{
		.cmd_name = "hwaddr",
		.cmd_help = "set/get mac address",
		.cmd_fn   = hwaddr_main
	},
	{
		.cmd_name = "route",
		.cmd_help = "show/manipulate the routing tables",
		.cmd_fn   = route_main
	},
	{
		.cmd_name = "nc",
		.cmd_help = "arbitrary TCP and UDP connections and listens",
		.cmd_fn   = nc_main
	},
	{ /* Terminating entry */ }
};

static void show_help(void)
{
	const struct shell_cmd *c;

	for (c = &cmd_list[0]; c->cmd_name; c++) {
		if (c->cmd_help)
			fprintf(stdout, "%-10s - %s\n", c->cmd_name, c->cmd_help);
	}
}

static void shell_run_cmd(int argc, char *const argv[])
{
	const struct shell_cmd *cmd;
	const char *argv0 = argv[0];

	if (!strcmp(argv0, "help") || *argv0 == '?')
		return show_help();

	for (cmd = &cmd_list[0]; cmd->cmd_name; cmd++) {
		if (!strcmp(argv0, cmd->cmd_name))
			return cmd->cmd_fn(argc, argv);
	}

	fprintf(stderr, "Command \"%s\" is unknown, try \"help\".\n", argv0);
}

static void __attribute__ ((unused))
arg_print(int argc, const char *argv[])
{
	int i;

	printf("argc: %d\n", argc);
	for (i = 0; i < argc; ++i)
		printf("%s\n", argv[i]);
}

#define ARGV_SIZE 8
#define ARGV_MAX (ARGV_SIZE - 1)

int shell_init(const char *prompt)
{
	char *argv[ARGV_SIZE], buf[1024], *cp;
	int argc;

	for (;;) {
		fprintf(stdout, "%s", prompt);
		if (!fgets(buf, sizeof(buf), stdin))
			break;

		argc = 0;
		cp = buf;

		/* Skip over any whitespace at start of string */
		while (isspace(*cp))
			cp++;

		while (*cp && argc < ARGV_MAX) {
			argv[argc++] = cp++;

			/* Skip over alpha-numeric chars to get next arg */
			while (isalnum(*cp) || ispunct(*cp))
				cp++;

			*cp++ = '\0';

			while (isspace(*cp))
				cp++;
		}

		argv[argc] = NULL;
		if (argc)
			shell_run_cmd(argc, argv);
	}
	fprintf(stdout, "\n");

	return 0;
}
