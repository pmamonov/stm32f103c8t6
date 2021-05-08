#include <stdio.h>
#include <stdlib.h>
#include <strtok.h>
#include <common.h>
#include <version.h>
#include <chat.h>
#include <bt.h>

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

static int cmd_bt_pair(struct iofun *rw)
{
	rw->puts("INITIALIZING BT DONGLE\r\n");
	bt_pair(rw->priv); /* executed in debug thread */
	rw->puts("PAIR THE DEVICE NOW\r\n");
}

static int cmd_bt_auto(struct iofun *rw)
{
	bt_auto(rw); /* executed in BT thread */
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
	{
		.name = "btp",
		.func = cmd_bt_pair,
	},
	{
		.name = "+BTSTATE:1",
		.func = cmd_bt_auto,
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
