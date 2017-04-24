#ifndef _SHELL_H
#define _SHELL_H

#include <stdio.h>

struct shell_struct {
	FILE *fp;
	int (*process_input)(struct shell_struct *sh);
	const char *prompt;
};

struct shell_struct *shell_init(FILE *fp, const char *prompt);
void shell_display_prompt(const struct shell_struct *sh);

#endif
