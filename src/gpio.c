#include <gpio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct gpio gpios[] = {
	[0] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_0,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[1] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_1,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[2] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_2,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[3] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_3,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[4] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_4,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[5] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_5,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[6] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_6,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
		.clk_cmd	= RCC_APB2PeriphClockCmd,
	},
	[7] = {
		.port		= GPIOA,
		.pin		= GPIO_Pin_7,
		.speed		= GPIO_Speed_10MHz,
		.mode		= GPIO_Mode_Out_PP,
		.clk		= RCC_APB2Periph_GPIOA,
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

void gpio_pwm_task(void *vpars)
{
	int ppwm = GPIO_PWM_PERIOD;
	int i, j;
	int d = 1, dd = 0, inc = 1;

	gpio_init();

	for (i = 0; i < ARRAY_SIZE(gpios); i++)
		gpios[i].pwm = ppwm / 2;

	while (1) {
		if (!(dd++ % 7)) {
			for (i = 0; i < ARRAY_SIZE(gpios); i++)
				gpios[i].pwm = i == d ? ppwm - 1 :
						i + inc == d ? ppwm / 2 :
						i + 2 * inc == d ? ppwm / 3 :
						i + 3 * inc == d ? ppwm / 4 :
						i + 4 * inc == d ? ppwm / 5 :
						i + 5 * inc == d ? ppwm / 6 :
						i + 6 * inc == d ? ppwm / 7 :
						0;
			if (d + 1 >= ARRAY_SIZE(gpios))
				inc = -1;
			if (d == 0)
				inc = 1;
			d += inc;
		}

		for (i = 0; i < ppwm; i++) {
			for (j = 0; j < ARRAY_SIZE(gpios); j++) {
				if (!i) {
					if (gpios[j].pwm)
						gpio_set_val(j, 1);
					else
						gpio_set_val(j, 0);
				}

				if (i > gpios[j].pwm)
					gpio_set_val(j, 0);
				
			}
			vTaskDelay(1);
		}
	}
}
