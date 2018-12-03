#ifndef __blink_h
#define __blink_h

#include "stm32f10x_gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#define SYS_LED_RCC RCC_APB2Periph_GPIOC
#define SYS_LED_GPIO GPIOC
#define SYS_LED_PIN GPIO_Pin_13

void blink_init();
void vBlinkTask(void *vpars);

static inline void blink_toggle()
{
	if (GPIO_ReadOutputDataBit(SYS_LED_GPIO, SYS_LED_PIN))
		GPIO_ResetBits(SYS_LED_GPIO, SYS_LED_PIN);
	else
		GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
}

static inline void blink_off()
{
		GPIO_SetBits(SYS_LED_GPIO, SYS_LED_PIN);
}

static inline void blink_on()
{
		GPIO_ResetBits(SYS_LED_GPIO, SYS_LED_PIN);
}

#endif
