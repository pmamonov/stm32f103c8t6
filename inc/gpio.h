#ifndef __GPIO_H__
#define __GPIO_H__

#include "FreeRTOS.h"
#include "task.h"
#include <stm32f10x_gpio.h>

struct gpio {
	GPIO_TypeDef *port;
	uint16_t pin;
	GPIOSpeed_TypeDef speed;
	GPIOMode_TypeDef mode;
	uint32_t clk;
	void (*clk_cmd)(uint32_t, FunctionalState);
	portTickType timeout;
};

int gpio_init();
int gpio_set_val(unsigned int, unsigned int);
int gpio_set_val_timeout(unsigned int, unsigned int, unsigned int);
int gpio_out_get(unsigned int);
int gpio_in_get(unsigned int);
void gpio_reset_task(void *);

#endif /* __GPIO_H__ */
