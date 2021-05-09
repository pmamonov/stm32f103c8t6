#include "stdio.h"
#include "stdlib.h"
#include "cdcio.h"
#include "string.h"
#include <gpio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define GPIO_POLL_DELAY	100

struct gpio gpios[] = {
	[0] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_1,
		.speed		= GPIO_Speed_2MHz,
		.mode		= GPIO_Mode_IPD,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[1] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_0,
		.speed		= GPIO_Speed_2MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
};

static void gpio_init_one(struct gpio *g)
{
	GPIO_InitTypeDef gi = {
		.GPIO_Pin = g->pin,
		.GPIO_Speed = g->speed,
		.GPIO_Mode = g->mode,
	};

	g->clk_cmd(g->clk, ENABLE);
	GPIO_Init(g->port, &gi);
}

int gpio_init()
{
	int i;

	for (i = 0; i < ARRAY_SIZE(gpios); i++)
		gpio_init_one(&gpios[i]);
	return 0;
}

int gpio_set_val(unsigned int i, unsigned int v)
{
	struct gpio *g = &gpios[i];

	if (i >= ARRAY_SIZE(gpios))
		return 1;

	GPIO_WriteBit(g->port, g->pin, v ? Bit_SET : Bit_RESET);
}

int gpio_set_val_timeout(unsigned int i, unsigned int v, unsigned int timeout)
{
	gpios[i].timeout = xTaskGetTickCount() + timeout;
	gpio_set_val(i, v);
};

int gpio_out_get(unsigned int i)
{
	struct gpio *g = &gpios[i];

	if (i >= ARRAY_SIZE(gpios))
		return -1;
	return !!GPIO_ReadOutputDataBit(g->port, g->pin);
}

int gpio_in_get(unsigned int i)
{
	struct gpio *g = &gpios[i];

	if (i >= ARRAY_SIZE(gpios))
		return -1;
	return !!GPIO_ReadInputDataBit(g->port, g->pin);
}

int gpio_in_get_all()
{
	int i, x = 0;

	for (i = 0; i < ARRAY_SIZE(gpios); i++)
		x |= gpio_in_get(i) << i;

	return x;
}

void gpio_reset_task(void *vpars)
{
	int i;

	while (1) {
		portTickType t = xTaskGetTickCount();

		for (i = 0; i < ARRAY_SIZE(gpios); i++) {
			if (gpio_out_get(i) > 0 && t >= gpios[i].timeout)
				gpio_set_val(i, 0);
		}
		vTaskDelay(10);
	}
}

void gpio_poll_task(void *vpars)
{
	int i;
	uint32_t cur, old = gpio_in_get_all();
	char s[32];

	while (1) {
		vTaskDelay(GPIO_POLL_DELAY);

		cur = gpio_in_get_all();

		if (cur ^ old) {
			sniprintf(s, sizeof(s), "gpio: 0x%08x\r\n", cur);
			cdc_write_buf(&cdc_out, s, strlen(s), 1);
			old = cur;
		}
	}
}
