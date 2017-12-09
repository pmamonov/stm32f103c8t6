#include "stdlib.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LiquidCrystal_I2C.h"
#include "lcd.h"

#define min(a, b)	((a) < (b) ? (a) : (b))

#define PERIOD	(configTICK_RATE_HZ / 8)

static char buf[SL * (SC + 1)];
static volatile int update;

char *lcd_getstr(int l)
{
	if (l >= SL)
		l = SL - 1;

	return &buf[l * (SC + 1)];
}

void lcd_setstr(int l, int off, char *s)
{
	int i, sl = min(strlen(s), SC - off);

	if (l >= SL)
		return;

	if (off >= SC)
		return;

	memcpy(&buf[l * (SC + 1) + off], s, sl);
	buf[l * (SC + 1) + SC] = 0;
	taskENTER_CRITICAL();
	update |= 1 << l;
	taskEXIT_CRITICAL();
}

static int lcd_init()
{
	int ret;

	ret = LCDI2C_init(PCF8574_ADDR, SC, SL);
	if (ret)
		return ret;
	ret = LCDI2C_backlight();
	if (ret)
		return ret;
	ret = LCDI2C_clear();
	return ret;
}

#define buf(l, c) (buf[(l) * (SC + 1) + (c)])

void lcd_task(void *vpars)
{
	int l, c;
	int ret = 1;
	portTickType t = xTaskGetTickCount();
	unsigned long long dmg[SL];


	srand(0);
	for (c = 0; c < SC; c++)
		for (l = 0; l < SL; l++)
			buf(l, c) = ' '; //rand() < RAND_MAX / 4 ? '*' : ' ';

	update = -1;

	while (1) {
		if (ret) {
			ret = lcd_init();
			update = -1;
		}
		if (ret)
			continue;

		memset(dmg, 0, sizeof(dmg));

		for (c = SC - 1; c > 0; c--)
			for (l = 0; l < SL; l++) {
				if (buf(l, c) != buf(l, c - 1)) {
					buf(l, c) = buf(l, c - 1);
					dmg[l] |= 1ull << c;
				}
			}

		for (l = 0; l < SL; l++) {
			char x = buf(l,1) == ' ' &&
					buf(l,2) == ' ' &&
					(l ? buf(l - 1, 0) == ' ' : 1) &&
					rand() % 100 < 20 ?
				'*' : ' ';
			if (buf(l, 0) != x) {
				buf(l, 0) = x;
				dmg[l] |= 1;
			}
		}


		for (l = 0; l < SL; l++)
			for (c = SC - 1; c >= 0; c--)
				if (buf(l, c) != ' ' && (dmg[l] >> c) & 1) {
					ret = LCDI2C_setCursor(c, l);
					ret |= LCDI2C_write(buf(l, c));
				}
		vTaskDelayUntil(&t, configTICK_RATE_HZ * 20 / 100);
		for (l = 0; l < SL; l++)
			for (c = SC - 1; c >= 0; c--)
				if (buf(l, c) == ' ' && (dmg[l] >> c) & 1) {
					ret = LCDI2C_setCursor(c, l);
					ret |= LCDI2C_write(buf(l, c));
				}
		vTaskDelayUntil(&t, configTICK_RATE_HZ * 20 / 100);
	}
}
