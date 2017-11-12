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
#include "co2.h"
#include "io.h"

#define USB_DP_PU_RCC	RCC_APB2Periph_GPIOB
#define USB_DP_PU_GPIO	GPIOB
#define USB_DP_PU_PIN	GPIO_Pin_9

static void usb_dp_pu()
{
	GPIO_InitTypeDef gpio_init = {
		.GPIO_Speed = GPIO_Speed_10MHz,
		.GPIO_Mode = GPIO_Mode_Out_PP,
		.GPIO_Pin = USB_DP_PU_PIN,
	};

	RCC_APB2PeriphClockCmd(USB_DP_PU_RCC, ENABLE);
	GPIO_Init(USB_DP_PU_GPIO, &gpio_init);
	GPIO_WriteBit(USB_DP_PU_GPIO, USB_DP_PU_PIN, Bit_SET);
}

int main(void)
{
	portBASE_TYPE err;
	char s[64];

	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	usb_dp_pu();

	flash_load();

	err = xTaskCreate(vBlinkTask, "blink", 64, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(vChatTask, "chat", 256, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(lcd_task, "lcd", 256, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(vADCTask, "adc", 256, NULL,
			  tskIDLE_PRIORITY + 2, NULL );

	err = xTaskCreate(co2_task, "co2", 256, NULL,
			  tskIDLE_PRIORITY + 1, NULL );

	err = xTaskCreate(io_task, "io", 256, NULL,
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
