#include <common.h>
#include <string.h>
#include <gpio.h>
#include <bt.h>

#define DEL	(2 * configTICK_RATE_HZ)

static int noauto;

void bt_disc()
{
	gpio_set_val(GPIO_BT_DISC, 1);
	vTaskDelay(configTICK_RATE_HZ / 10);
	gpio_set_val(GPIO_BT_DISC, 0);
}

int bt_pair(struct iofun *rw)
{
	noauto = 1;

	bt_disc();

	vTaskDelay(2 * DEL);
	rw->puts("\r\n+STAUTO=0\r\n");
	vTaskDelay(DEL);
	rw->puts("\r\n+STWMOD=0\r\n");
	vTaskDelay(DEL);
	rw->puts("\r\n+STNA=BTEST\r\n");
	vTaskDelay(DEL);
	rw->puts("\r\n+STPIN=1337\r\n");
	vTaskDelay(DEL);
	rw->puts("\r\n+STOAUT=1\r\n");
	vTaskDelay(DEL);
	rw->puts("\r\n+INQ=1\r\n");
	vTaskDelay(DEL);

	return 0;
}

int bt_auto(struct iofun *rw)
{
	if (noauto)
		return 0;
	noauto = 1;
	rw->puts("\r\n+STAUTO=1\r\n");
}
