#include <common.h>
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "stdlib.h"
#include "cdcio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "strtok.h"
#include "blink.h"
#include "adc.h"
#include "chat.h"
#include "lcd.h"
#include "flash.h"
#include "uart.h"
#include "gpio.h"
#include <bt.h>

#define BT_DEBUG

int uart2_puts(char *s)
{
	return uart_puts(1, s);
}

int uart2_readline(char *s, unsigned len)
{
	return uart_readline(1, s, len);
}


int uart3_puts(char *s)
{
#ifdef BT_DEBUG
	uart2_puts("BT > ");
	uart2_puts(s);
#endif
	return uart_puts(2, s);
}

int uart3_readline(char *s, unsigned len)
{
	int ret = uart_readline(2, s, len);
#ifdef BT_DEBUG
	uart2_puts("BT < ");
	uart2_puts(s);
#endif
	return ret;
}

struct iofun uart3_rw = {
	.puts = uart3_puts,
	.readline = uart3_readline,
};

struct iofun uart2_rw = {
	.puts = uart2_puts,
	.readline = uart2_readline,
	.priv = &uart3_rw,
};

int main(void)
{
	portBASE_TYPE err;
	char s[64];

	gpio_init();

	uart_init(1, 115200); /* debug */

	uart_init(2, 38400); /* BT */

	flash_load();

	err = xTaskCreate(vBlinkTask, "blink", 64, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(vChatTask, "chat", 256, &uart2_rw,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(vChatTask, "chat-bt", 256, &uart3_rw,
			  tskIDLE_PRIORITY + 1, NULL );

	vTaskStartScheduler();

	while(1)
		;
}

void vApplicationStackOverflowHook(xTaskHandle xTask,
			           signed portCHAR *pcTaskName )
{
	while(1)
		;
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
/* User can add his own implementation to report the file name and line number,
 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
		;
}
#endif
