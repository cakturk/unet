#ifndef UNET_H_
#define UNET_H_

#include <stdio.h>

#if DEBUG
#define pr_dbg(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)
#define fpr_dbg(fp, fmt, ...) \
	fprintf(fp, fmt, ##__VA_ARGS__)
#define pr_inf(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)
#define fpr_inf(fp, fmt, ...) \
	fprintf(fp, fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#define fpr_dbg(fp, fmt, ...)
#define pr_inf(fmt, ...)
#define fpr_inf(fp, fmt, ...)
#endif

#endif /* end of include guard: UNET_H_ */
