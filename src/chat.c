#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"
#include "counter.h"

#define PROMPT	"> "

enum {
	CMD_HELP = 0,
	CMD_ECHO,
	CMD_VER,
	CMD_DATE,
	CMD_DISP,
	CMD_CNT,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	[CMD_HELP] =	"help",
	[CMD_ECHO] =	"echo",
	[CMD_VER] =	"ver",
	[CMD_DATE] =	"date",
	[CMD_DISP] =	"disp",
	[CMD_CNT] =	"cnt",
};

void vChatTask(void *vpars)
{
	char s[64];
	char cmd[64];
	char *c;
	char *tk;
	int i = 0;
	int echo = 1;

	while (1) {
		if (echo)
			cdc_write_buf(&cdc_out, PROMPT, sizeof(PROMPT) - 1, 1);
		memset(cmd, 0, sizeof(cmd));
		c = cmd;

		while (1) {
			i = cdc_read_buf(&cdc_in, c, 1);
			if (i) {
				if (echo)
					cdc_write_buf(&cdc_out, c, 1, 1);
			} else {
				vTaskDelay(10);
				continue;
			}
			if (*c == '\r') {
				if (echo)
					cdc_write_buf(&cdc_out, "\n", 1, 1);
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

				cdc_write_buf(&cdc_out, _s, strlen(_s), 1);
				cdc_write_buf(&cdc_out, "\r\n", 2, 1);
			}

		} else if (strcmp(tk, cmd_list[CMD_DATE]) == 0) {
			sniprintf(s, sizeof(s), "%d\r\n", xTaskGetTickCount());

		} else if (strcmp(tk, cmd_list[CMD_DISP]) == 0) {
			int l, o, i;

			tk = _strtok(NULL, " \n\r");
			if (!tk) {
				int l;
				char *s;

				for (l = 0; l < SL; l++) {
					s = lcd_getstr(l);
					cdc_write_buf(&cdc_out, s, strlen(s), 1);
					cdc_write_buf(&cdc_out, "\n\r", 2, 1);
				}
				goto out;
			}
			l = atoi(tk);

			tk = _strtok(NULL, " \n\r");
			if (!tk) {
				sniprintf(s, sizeof(s), "E: no offset\r\n");
				goto out;
			}
			o = atoi(tk);

			tk = _strtok(NULL, "\n\r");
			if (!tk) {
				sniprintf(s, sizeof(s), "E: no string\r\n");
				goto out;
			}

			for (i = 0; i < strlen(tk); i++)
				if (tk[i] == '_')
					tk[i] = ' ';

			lcd_setstr(l, o, tk);

		} else if (strcmp(tk, cmd_list[CMD_CNT]) == 0) {
			for (i = 0; i < CNT_NUM; i++) {
				sniprintf(s, sizeof(s), "%d ", cnt_get(i));
				cdc_write_buf(&cdc_out, s, strlen(s), 1);
			}
			sniprintf(s, sizeof(s), "\r\n");
		} else
			sniprintf(s, sizeof(s), "E: try `help`\r\n");
out:
		cdc_write_buf(&cdc_out, s, strlen(s), 1);
	}
}

