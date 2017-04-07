#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

static void subcmd(int argc, char *const argv[])
{
	int bflag, ch;
	int daggerset = 0, sword = 0;

	/* Reinitialize 'getopt'. Note that to re-initialize getopt on BSDs
	 * the variable 'optreset' must also be set to 1.
	 */
	optind = 1;

	/* options descriptor */
	const struct option longopts[] = {
		{ "buffy",      no_argument,            NULL,           'b' },
		{ "fluoride",   required_argument,      NULL,           'f' },
		{ "daggerset",  no_argument,            &daggerset,     1 },
		{ "sword",      no_argument,            &sword,         1 },
		{ /* Terminating entry */ }
	};

	bflag = 0;
	opterr = 0;
	while ((ch = getopt_long(argc, argv, "bf:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'b':
			bflag = 1;
			printf("bflag is set: %d!\n", bflag);
			break;
		case 'f':
			printf("option f: %s\n", optarg);
			break;
		case 0:
			if (daggerset) {
				fprintf(stderr,"Buffy will use her dagger to "
					"apply fluoride to dracula's teeth\n");
			}
			if (sword) {
				fprintf(stderr,"Buffy will use magic sword\n");
			}
			break;
		default:
			printf("usage invoked\n");
		}
	}
	argc -= optind;
	argv += optind;
}

struct shell_cmd {
	const char *cmd_name;
	const char *cmd_help;
	void (*cmd_fn)(int argc, char *const argv[]);
};

void ip_cmd_main(int argc, char *const argv[]);
void hwaddr_main(int argc, char *const *argv);
void route_main(int argc, char *const argv[]);

static const struct shell_cmd cmd_list[] = {
	{
		.cmd_name = "foo",
		.cmd_help = "Does nothing just foo!",
		.cmd_fn   = subcmd
	},
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

/* int main(int argc, char *argv[]) */
/* { */
/* 	/1* printf("optind initial value: %d\n", optind); *1/ */
/* 	if (argc > 1) { */
/* 		const char *cmd = argv[1]; */

/* 		printf("subcommand is %s\n", cmd); */

/* 		optind = 2; */
/* 		subcmd(argc, argv); */
/* 		optind = 2; */
/* 		subcmd2(argc, argv); */
/* 	} */
/* 	shell_init("unet-shell-> "); */

/* 	return 0; */
/* } */
