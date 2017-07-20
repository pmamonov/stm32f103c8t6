#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "stm32f10x_gpio.h"
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

int main(void)
{
	portBASE_TYPE err;
	char s[64];

	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

	flash_load();

	err = xTaskCreate(vBlinkTask, "blink", 64, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(vChatTask, "chat", 256, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(lcd_task, "lcd", 256, NULL,
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
