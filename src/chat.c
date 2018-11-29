#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"

#define PROMPT	"> "

#define RX_TIMEOUT	100

char rx_buf[4096];

enum {
	CMD_HELP = 0,
	CMD_ECHO,
	CMD_VER,
	CMD_DATE,
	CMD_DISP,
	CMD_RX,
	CMD_RXDUMP,
	CMD_TX,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	[CMD_HELP] =	"help",
	[CMD_ECHO] =	"echo",
	[CMD_VER] =	"ver",
	[CMD_DATE] =	"date",
	[CMD_DISP] =	"disp",
	[CMD_RX] =	"rx",
	[CMD_RXDUMP] =	"rxdump",
	[CMD_TX] =	"tx",
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

		} else if (strcmp(tk, cmd_list[CMD_RXDUMP]) == 0) {
			int i;

			for (i = 0; i < sizeof(rx_buf); i++) {
				if (!(i % 16)) {
					sniprintf(s, sizeof(s), "\r\n%04x:", i);
					cdc_write_buf(&cdc_out, s, strlen(s), 1);
				}
				sniprintf(s, sizeof(s), " %02x", rx_buf[i]);
				cdc_write_buf(&cdc_out, s, strlen(s), 1);
			}
			sniprintf(s, sizeof(s), "\r\n");
		} else if (strcmp(tk, cmd_list[CMD_TX]) == 0) {
			int i, bs = 1 << 8, n = 0x10, t;

			tk = _strtok(NULL, "\n\r");
			if (tk)
				bs = atoi(tk);

			tk = _strtok(NULL, "\n\r");
			if (tk)
				n = atoi(tk);

			t = xTaskGetTickCount();
			for (i = 0; i < n; i++)
				cdc_write_buf(&cdc_out, &rx_buf[(i * bs) % sizeof(rx_buf)], bs, 1);

			sniprintf(s, sizeof(s), "%d\r\n", xTaskGetTickCount() - t);

		} else if (strcmp(tk, cmd_list[CMD_RX]) == 0) {
			int r = 0, len = 0, t, tout;

			tk = _strtok(NULL, "\n\r");
			if (tk)
				len = atoi(tk);

			cdc_write_buf(&cdc_out, "\r\n", 2, 1);

			t = xTaskGetTickCount();
			tout = t + RX_TIMEOUT;
			while (r < len) {
				int d = cdc_read_buf(&cdc_in, &rx_buf[r % sizeof(rx_buf)], len - r);

				r += d;
				if (d)
					tout = xTaskGetTickCount() + RX_TIMEOUT;
				if (xTaskGetTickCount() > tout)
					break;
			}
			sniprintf(s, sizeof(s), "%d %d\r\n", r, xTaskGetTickCount() - t);

		} else
			sniprintf(s, sizeof(s), "E: try `help`\r\n");
out:
		cdc_write_buf(&cdc_out, s, strlen(s), 1);
	}
}

