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

void blink_toggle() {
	if (GPIO_ReadOutputDataBit(SYS_LED_GPIO, SYS_LED_PIN))
		GPIO_ResetBits(SYS_LED_GPIO, SYS_LED_PIN);
	else
		GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
}

static void pwm1(int p, int d)
{
	if (d) {
		GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
		vTaskDelay(d);
	}
	if (p - d) {
		GPIO_ResetBits(SYS_LED_GPIO, SYS_LED_PIN);
		vTaskDelay(p - d);
	}
}

static void npwm(int n, int p, int d)
{
	while (n--)
		pwm1(p, d);
}

#define P	10
void vBlinkTask(void *vpars)
{
	int i;

	blink_init();
	while (1) {
		for (i = 0; i < P; i++)
			npwm(10, P, i);
		for (i = 0; i < P; i++)
			npwm(10, P, P-i);
	}
}
