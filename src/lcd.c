#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LiquidCrystal_I2C.h"
#include "lcd.h"
#include "cdcio.h"

#define min(a, b)	((a) < (b) ? (a) : (b))

#define PERIOD	(configTICK_RATE_HZ / 5)

static char buf[SL * (SC + 1)];
static volatile int update;
static volatile int dump;

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

void lcd_dump()
{
	int l;

	cdc_write_buf(&cdc_out, "\n\r", 2, 0);
	for (l = 0; l < SL; l++) {
		char *s = lcd_getstr(l);
		cdc_write_buf(&cdc_out, s, strlen(s), 0);
		cdc_write_buf(&cdc_out, "\n\r", 2, 0);
	}
}

void lcd_dump_toggle()
{
	dump = !dump;
}

static void lcd_err_log(int err, int block)
{
	cdc_write_buf(&cdc_out, "LCD: ", 5, block);
	switch (err) {
	case EBUSY:
		cdc_write_buf(&cdc_out, "EBUSY", 5, block);
		break;
	case EMASTER:
		cdc_write_buf(&cdc_out, "EMASTER", 7, block);
		break;
	case ETRANS:
		cdc_write_buf(&cdc_out, "ETRANS", 6, block);
		break;
	case ERECV:
		cdc_write_buf(&cdc_out, "ERECV", 5, block);
		break;
	case EMBTRANS:
		cdc_write_buf(&cdc_out, "EMBTRANS", 8, block);
		break;
	default:
		cdc_write_buf(&cdc_out, "UNKNOWN", 7, block);
		break;
	}
	cdc_write_buf(&cdc_out, "\n\r", 2, block);
}

void lcd_task(void *vpars)
{
	int l;
	int i;
	int ret = 1;
	portTickType t = xTaskGetTickCount();

	dump = 1;
	for (l = 0; l < SL; l++)
		lcd_setstr(l, 0, "                    ");


	while (1) {
		vTaskDelayUntil(&t, PERIOD);
		t = xTaskGetTickCount();

		if (dump && !i)
			lcd_dump();
		i = (i + 1) % 5;

		if (!update)
			continue;

		if (ret) {
			lcd_err_log(ret, 0);
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
		update = 0;
	}
}
