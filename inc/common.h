#ifndef __COMMON_H__
#define __COMMON_H__

#include <FreeRTOS.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct iofun {
	void *priv;
	int (*puts)(char *s);
	int (*readline)(char *s, unsigned len);
};

#endif /* __COMMON_H__ */
