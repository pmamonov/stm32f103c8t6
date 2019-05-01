#include "stdio.h"
#include "stdlib.h"
#include "chat.h"
#include "strtok.h"
#include "version.h"
#include "lcd.h"
#include "i2c.h"
#include "pwm.h"

#define PROMPT	"> "

enum {
	CMD_HELP = 0,
	CMD_ECHO,
	CMD_VER,
	CMD_DATE,
	CMD_DISP,
	CMD_I2CDET,
	CMD_CO2,
	CMD_PWM,
	CMD_TEMP,

	CMD_LAST
};

char *cmd_list[CMD_LAST] = {
	[CMD_HELP] =	"help",
	[CMD_ECHO] =	"echo",
	[CMD_VER] =	"ver",
	[CMD_DATE] =	"date",
	[CMD_DISP] =	"disp",
	[CMD_I2CDET] =	"i2cdetect",
	[CMD_CO2] =	"co2",
	[CMD_PWM] =	"pwm",
	[CMD_TEMP] =	"temp",
};

void vChatTask(void *vpars)
{
	char s[64];
	char cmd[64];
	char *c;
	char *tk;
	int i = 0;
	int echo = 1;

	pwm_init_all(2000);

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

		} else if (strcmp(tk, cmd_list[CMD_I2CDET]) == 0) {
			int i = 0, r = 1000, a;

			tk = _strtok(NULL, " \n\r");
			if (tk)
				i = atoi(tk);

			tk = _strtok(NULL, " \n\r");
			if (tk)
				r = atoi(tk);

			i2c_init(i, r);
			for (a = 8; a < 0x78; a++) {
				int rc = i2c_start(i, I2C_TX, a);

				i2c_stop(i);

				if (!rc) {
					sniprintf(s, sizeof(s), "@ 0x%02x\r\n", a);
					cdc_write_buf(&cdc_out, s, strlen(s), 1);
				}
			}
			sniprintf(s, sizeof(s), "DONE\r\n");
		} else if (strcmp(tk, cmd_list[CMD_CO2]) == 0) {
			uint8_t cmd[] = {0x04, 0x13, 0x8b, 0x00, 0x01};
			uint8_t resp[4];
			int rc;

			rc = i2c_xmit(0, 0x15, cmd, sizeof(cmd));
			if (rc != sizeof(cmd)) {
				sniprintf(s, sizeof(s), "E: xmit: %d\r\n", rc);
				goto out;
			}
			vTaskDelay(100);
			memset(resp, 0, sizeof(resp));
			rc = i2c_recv(0, 0x15, resp, sizeof(resp));
			if (rc != sizeof(resp)) {
				sniprintf(s, sizeof(s), "E: recv: %d\r\n", rc);
				goto out;
			}
			sniprintf(s, sizeof(s), "%02x %02x %d\r\n",
				resp[0], resp[1], ((unsigned)resp[2] << 8) | resp[3]);

		} else if (strcmp(tk, cmd_list[CMD_PWM]) == 0) {
			unsigned int c, d, dc;
			char *help = "pwm <PWM> <DELAY_MS> <DUTY_CYCLE_%>\r\n";

			tk = _strtok(NULL, " \n\r");
			if (!tk) {
				memcpy(s, help, strlen(help) + 1);
				goto out;
			}
			c = atoi(tk);
			if (c >= pwm_count()) {
				sniprintf(s, sizeof(s), "E: PWM%d unavailable\r\n", c);
				goto out;
			}

			tk = _strtok(NULL, " \n\r");
			if (!tk) {
				memcpy(s, help, strlen(help) + 1);
				goto out;
			}
			d = atoi(tk);

			tk = _strtok(NULL, " \n\r");
			if (!tk) {
				memcpy(s, help, strlen(help) + 1);
				goto out;
			}
			dc = atoi(tk);

			if (dc > 100) {
				sniprintf(s, sizeof(s), "E: duty cycle %d%% > 100%%\r\n", dc);
				goto out;
			}

			if (d > 0) {
				pwm_set(c, 100);
				vTaskDelay(d);
			}
			pwm_set(c, dc);
		} else if (strcmp(tk, cmd_list[CMD_TEMP]) == 0) {
			unsigned addr = 0x48;
			uint8_t resp[4];
			int rc;

			rc = i2c_xmit(0, addr, "\x03\x80", 2);
			if (rc <= 0) {
				sniprintf(s, sizeof(s), "E: xmit 1: %d\r\n", rc);
				goto out;
			}
			rc = i2c_xmit(0, addr, "\x00", 1);
			if (rc <= 0) {
				sniprintf(s, sizeof(s), "E: xmit 2: %d\r\n", rc);
				goto out;
			}
			memset(resp, 0, sizeof(resp));
			rc = i2c_recv(0, addr, resp, 3); /* FIXME: 3 */
			if (rc <= 0) {
				sniprintf(s, sizeof(s), "E: recv: %d\r\n", rc);
				goto out;
			}
			rc = ((unsigned)resp[0] << 8) | resp[1];
			sniprintf(s, sizeof(s), "t: %x %d\r\n",
				  rc , rc * 1000 / 128);

		} else
			sniprintf(s, sizeof(s), "E: try `help`\r\n");
out:
		cdc_write_buf(&cdc_out, s, strlen(s), 1);
	}
}

