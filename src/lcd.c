#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LiquidCrystal_I2C.h"
#include "lcd.h"

#define min(a, b)	((a) < (b) ? (a) : (b))

#define PERIOD	(configTICK_RATE_HZ * 10)

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

static void symtab(int i)
{
	int l, c;
	char x;

	for (l = 0; l < SL; l++)
		for (c = 0; c < SC; c++)
			if (i < 0x100)
				buf(l, c) = i++;
			else
				buf(l, c) = ' ';
}

void lcd_task(void *vpars)
{
	int l, i = 0;
	int ret = 1;
	portTickType t = xTaskGetTickCount();


	while (1) {
		vTaskDelayUntil(&t, PERIOD);

		symtab(i);
		i += SL * SC;
		if (i > 0xff)
			i = 0;
		update = -1;

		if (ret) {
			ret = lcd_init();
			update = -1;
		}
		if (ret)
			continue;

		for (l = 0; l < SL; l++) {
			if (!(update & (1 << l)))
				continue;
			ret = LCDI2C_setCursor(0, l);
			if (ret)
				break;
			ret = LCDI2C_write_String(&buf[l * (SC + 1)]);
			if (ret)
				break;
		}
		if (!ret)
			update = 0;
	}
}
