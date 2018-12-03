#include "blink.h"

void blink_init()
{
	GPIO_InitTypeDef sGPIOinit;

	RCC_APB2PeriphClockCmd(SYS_LED_RCC, ENABLE);
	sGPIOinit.GPIO_Mode = GPIO_Mode_Out_PP;
	sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
	sGPIOinit.GPIO_Pin = SYS_LED_PIN;
	GPIO_Init(SYS_LED_GPIO, &sGPIOinit);
	GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
}

void vBlinkTask(void *vpars)
{
	blink_init();
	while (1) {
		blink_toggle();
		vTaskDelay(200);
	}
}
