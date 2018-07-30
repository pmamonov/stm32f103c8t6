#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"
#include "ad779x_stm32.h"
#include <stm32f10x_spi.h>
#include <gpio.h>

#define PROMPT	"> "

enum {
	CMD_HELP = 0,
	CMD_VER,
	CMD_DATE,
	CMD_DISP,
	CMD_ADC_INIT,
	CMD_ADC,
	CMD_SPI_TEST,
	CMD_SPI,
	CMD_SPI_CS,
	CMD_GPIO,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	"help",
	"ver",
	"date",
	"disp",
	"adc_init",
	"adc",
	"spi_test",
	"spi",
	"spi_cs",
	"gpio",
};

void vChatTask(void *vpars)
{
	char s[64];
	char cmd[64];
	char *c;
	char *tk;
	int i = 0;
	int adc_ret = -1;

	gpio_init();

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

		} else if (strcmp(tk, cmd_list[CMD_ADC_INIT]) == 0) {
			adc_ret = ad779x_stm32_init();
			sniprintf(s, sizeof(s), "ret=%d, spi=%d, rx=%02x\r\n",
				adc_ret, spi_err, ADCDevice.dbg);
		} else if (strcmp(tk, cmd_list[CMD_ADC]) == 0) {
			int i = 0;
			long x;

			if (adc_ret) {
				sniprintf(s, sizeof(s),
					"E: ADC init failed %d\r\n", adc_ret);
				goto out;
			}
			tk = _strtok(NULL, " \n\r");
			if (tk)
				i = atoi(tk);
			x = ad779x_stm32_read(i);
			if (x < 0)
				sniprintf(s, sizeof(s), "E: st=%x\r\n", ADCDevice.dbg);
			else
				sniprintf(s, sizeof(s), "%d 0x%x (st=%x)\r\n", i, x, ADCDevice.dbg);
		} else if (strcmp(tk, cmd_list[CMD_SPI_CS]) == 0) {
			int en = GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_4);

			tk = _strtok(NULL, " \n\r");
			if (tk)
				en = atoi(tk);

			sniprintf(s, sizeof(s), "cs %d\r\n", !!en);

			if (en)
				GPIO_SetBits(GPIOA, GPIO_Pin_4);
			else
				GPIO_ResetBits(GPIOA, GPIO_Pin_4);

		} else if (strcmp(tk, cmd_list[CMD_SPI]) == 0) {
			int x = 0;
			char *_s = s;

			*_s = 0;

			tk = _strtok(NULL, " \n\r");
			if (tk)
				x = strtol(tk, NULL, 0);

			_s += sniprintf(_s, sizeof(s) - strlen(_s), "tx=%x ", x);

			SPI_I2S_ReceiveData(AD779X_SPI);
			spi_tx(x);
			x = SPI_I2S_ReceiveData(AD779X_SPI);
			sniprintf(_s, sizeof(s) - strlen(_s), "rx=%x\r\n", x);
		} else if (strcmp(tk, cmd_list[CMD_SPI_TEST]) == 0) {
			int n = 10000, x = 0xaa;

			while (n--) {
				spi_err = 0;
				spi_tx(x);
				if (spi_err) {
					sniprintf(s, sizeof(s), "E: %d\r\n", spi_err);
					goto out;
				}
			}
		} else if (strcmp(tk, cmd_list[CMD_GPIO]) == 0) {
			int i = 0, v;

			tk = _strtok(NULL, " \n\r");
			if (tk)
				i = atoi(tk);
			tk = _strtok(NULL, " \n\r");
			if (tk) {
				v = !!atoi(tk);
				gpio_set_val(i, v);
			} else {
				v = gpio_out_get(i);
			}
			sniprintf(s, sizeof(s), "%d: %d\r\n", i, v);
		} else
			sniprintf(s, sizeof(s), "E: try `help`\r\n");
out:
		cdc_write_buf(&cdc_out, s, strlen(s), 1);
	}
}

