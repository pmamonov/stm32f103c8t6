#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"

#define PROMPT	"> "

enum {
	CMD_HELP = 0,
	CMD_ECHO,
	CMD_VER,
	CMD_DATE,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	[CMD_HELP] =	"help",
	[CMD_ECHO] =	"echo",
	[CMD_VER] =	"ver",
	[CMD_DATE] =	"date",
};

void vChatTask(void *vpars)
{
	char s[64];
	char cmd[64];
	char *c;
	char *tk;
	int i = 0;
	int echo = 1;
	struct chat_rw_funcs *rw = vpars;

	while (1) {
		if (echo)
			rw->write(PROMPT, sizeof(PROMPT) - 1);
		memset(cmd, 0, sizeof(cmd));
		c = cmd;

		while (1) {
			i = rw->read(c, 1);
			if (i) {
				if (echo)
					rw->write(c, 1);
			} else {
				vTaskDelay(10);
				continue;
			}
			if (*c == '\r') {
				if (echo)
					rw->write("\n", 1);
				break;
			}
			if (*c == 8) { /* backspace */
				*c = 0;
				if (c > cmd)
					c -= 1;
				continue;
			}
			if (c + 1 < cmd + sizeof(cmd))
				c += 1;
		};

		sniprintf(s, sizeof(s), "\r\n");
		tk = _strtok(cmd, " \n\r");

		if (strcmp(tk, cmd_list[CMD_VER]) == 0) {
			sniprintf(s, sizeof(s), "%s\r\n", __VERSION);

		} else if (strcmp(tk, cmd_list[CMD_ECHO]) == 0) {
			int i = 1;

			tk = _strtok(NULL, " \n\r");
			if (tk)
				i = atoi(tk);

			echo = !!i;
			sniprintf(s, sizeof(s), "echo %d\r\n", echo);
		} else if (strcmp(tk, cmd_list[CMD_HELP]) == 0) {
			int i;

			for (i = 0; i < CMD_LAST; i++) {
				char *_s = cmd_list[i];

				rw->write(_s, strlen(_s));
				rw->write("\r\n", 2);
			}

		} else if (strcmp(tk, cmd_list[CMD_DATE]) == 0) {
			sniprintf(s, sizeof(s), "%d\r\n", xTaskGetTickCount());
		} else
			sniprintf(s, sizeof(s), "E: try `help`\r\n");
out:
		rw->write(s, strlen(s));
	}
}

