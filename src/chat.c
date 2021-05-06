#include <stdio.h>
#include <stdlib.h>
#include <strtok.h>
#include <common.h>
#include <version.h>
#include <chat.h>

struct cmd {
	char *name;
	int (*func)(struct iofun *rw);
};

static char ifs[] = " \n\r\t";

static int ver(struct iofun *rw)
{
	char s[32];

	sniprintf(s, sizeof(s), "%s\r\n", __VERSION);
	rw->puts(s);
	return 0;
}

static int test(struct iofun *rw)
{
	char *tk;
	int i = -1;
	char s[32];

	tk = _strtok(NULL, ifs);
	if (tk)
		i = atoi(tk);
	sniprintf(s, sizeof(s), "test %d\r\n", i);
	rw->puts(s);
	return 0;
}

static struct cmd cmd_list[] = {
	{
		.name = "ver",
		.func = ver,
	},
	{
		.name = "test",
		.func = test,
	},
};

void vChatTask(void *vpars)
{
	char cmd[64];
	char *tk;
	int i;
	struct iofun *rw = vpars;

	while (1) {
		i = rw->readline(cmd, sizeof(cmd));

		if (i <= 0) {
			vTaskDelay(10);
			continue;
		}

		tk = _strtok(cmd, ifs);
		for (i = 0; i < ARRAY_SIZE(cmd_list); i++)
			if (!strcmp(cmd_list[i].name, tk))
				cmd_list[i].func(rw);
	}
}
