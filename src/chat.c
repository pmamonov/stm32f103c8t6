#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"
#include "ad779x_stm32.h"
#include <stm32f10x_spi.h>

#define PROMPT	"> "

enum {
	CMD_HELP = 0,
	CMD_VER,
	CMD_DATE,
	CMD_DISP,
	CMD_ADC_INIT,
	CMD_ADC,
	CMD_SPI,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	"help",
	"ver",
	"date",
	"disp",
	"adc_init",
	"adc",
	"spi",
};

void vChatTask(void *vpars)
{
	char s[64];
	char cmd[64];
	char *c;
	char *tk;
	int i = 0;
	int adc_ret = -1;

	while (1) {
		cdc_write_buf(&cdc_out, PROMPT, sizeof(PROMPT) - 1, 1);
		memset(cmd, 0, sizeof(cmd));
		c = cmd;

		while (1) {
			i = cdc_read_buf(&cdc_in, c, 1);
			if (i)
				cdc_write_buf(&cdc_out, c, 1, 1);
			else {
				vTaskDelay(10);
				continue;
			}
			if (*c == '\r') {
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

		sniprintf(s, sizeof(s), "OK\r\n");
		tk = _strtok(cmd, " \n\r");

		if (strcmp(tk, cmd_list[CMD_VER]) == 0) {
			sniprintf(s, sizeof(s), "%s\r\n", __VERSION);

		} else if (strcmp(tk, cmd_list[CMD_HELP]) == 0) {
			int i;
			char *_s = s;

			for (i = 0; i < CMD_LAST; i++)
				_s += sniprintf(_s, sizeof(s) - strlen(s),
						"%s\r\n", cmd_list[i]);

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

		} else if (strcmp(tk, cmd_list[CMD_ADC_INIT]) == 0) {
			adc_ret = ad779x_stm32_init();
			sniprintf(s, sizeof(s), "%d / %d\r\n", adc_ret, spi_err);
		} else if (strcmp(tk, cmd_list[CMD_ADC]) == 0) {
			int i = 0;
			unsigned long x;

			if (adc_ret) {
				sniprintf(s, sizeof(s),
					"E: ADC init failed %d\r\n", adc_ret);
				goto out;
			}
			tk = _strtok(NULL, " \n\r");
			if (tk)
				i = atoi(tk);
			x = ad779x_stm32_read(i);
			sniprintf(s, sizeof(s), "%d 0x%x\r\n", i, x);
		} else if (strcmp(tk, cmd_list[CMD_SPI]) == 0) {
			int n = 10000, x = 0xaa;

			while (n--) {
				spi_err = 0;
				spi_tx(x);
				if (spi_err) {
					sniprintf(s, sizeof(s), "E: %d\r\n", spi_err);
					goto out;
				}
			}
		} else
			sniprintf(s, sizeof(s), "E: what?\r\n");
out:
		cdc_write_buf(&cdc_out, s, strlen(s), 1);
	}
}

