#include <gpio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct gpio gpios[] = {
	[0] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_10,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[1] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_11,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[2] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_12,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[3] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_13,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[4] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_14,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOB,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[5] = {
		.port		= GPIOB,
		.pin		= GPIO_Pin_15,
		.speed		= GPIO_Speed_10MHz,
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
