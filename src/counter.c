#include "counter.h"

#define EDGE !=
#define CNT_DEL	50

struct gpio_pin {
	GPIO_TypeDef *gpio;
	uint16_t pin;
	uint32_t count;
	uint8_t new;
	uint8_t prev;
	uint32_t rcc;
};

static struct gpio_pin gpios[CNT_NUM] = {
	[0] = {.gpio = GPIOB, .pin = GPIO_Pin_7, .rcc = RCC_APB2Periph_GPIOB},
	[1] = {.gpio = GPIOA, .pin = GPIO_Pin_8, .rcc = RCC_APB2Periph_GPIOA},
	[2] = {.gpio = GPIOB, .pin = GPIO_Pin_13, .rcc = RCC_APB2Periph_GPIOB},
	[3] = {.gpio = GPIOB, .pin = GPIO_Pin_12, .rcc = RCC_APB2Periph_GPIOB},
	[4] = {.gpio = GPIOB, .pin = GPIO_Pin_8, .rcc = RCC_APB2Periph_GPIOB},
	[5] = {.gpio = GPIOA, .pin = GPIO_Pin_9, .rcc = RCC_APB2Periph_GPIOA},
	[6] = {.gpio = GPIOB, .pin = GPIO_Pin_14, .rcc = RCC_APB2Periph_GPIOB},
	[7] = {.gpio = GPIOB, .pin = GPIO_Pin_10, .rcc = RCC_APB2Periph_GPIOB},
	[8] = {.gpio = GPIOB, .pin = GPIO_Pin_9, .rcc = RCC_APB2Periph_GPIOB},
	[9] = {.gpio = GPIOB, .pin = GPIO_Pin_6, .rcc = RCC_APB2Periph_GPIOB},
	[10] = {.gpio = GPIOB, .pin = GPIO_Pin_15, .rcc = RCC_APB2Periph_GPIOB},
	[11] = {.gpio = GPIOB, .pin = GPIO_Pin_11, .rcc = RCC_APB2Periph_GPIOB},
};

xSemaphoreHandle gpios_lock = NULL;

unsigned int cnt_get(int i)
{
	unsigned int ret = 0;

	if (i < CNT_NUM) {
		xSemaphoreTake(gpios_lock, portMAX_DELAY);
		ret = gpios[i].count;
		xSemaphoreGive(gpios_lock);
	}
	return ret;
}

void cnt_reset(int i)
{
	if (i < CNT_NUM) {
		xSemaphoreTake(gpios_lock, portMAX_DELAY);
		gpios[i].count = 0;
		xSemaphoreGive(gpios_lock);
	}
}

void cnt_task(void* par)
{
	int i;
	GPIO_InitTypeDef sGPIOinit;

	//TODO: init gpio

	for (i = 0; i < CNT_NUM; i++) {
		gpios[i].count = 0;
		RCC_APB2PeriphClockCmd(gpios[i].rcc, ENABLE);
		sGPIOinit.GPIO_Mode = GPIO_Mode_IPU;
		sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
		sGPIOinit.GPIO_Pin = gpios[i].pin;
		GPIO_Init(gpios[i].gpio, &sGPIOinit);
	}

	gpios_lock = xSemaphoreCreateMutex();

	while (1) {
		xSemaphoreTake(gpios_lock, portMAX_DELAY);
		for (i = 0; i < CNT_NUM; i++) {
			gpios[i].new = GPIO_ReadInputDataBit(gpios[i].gpio,
							   gpios[i].pin);
			if (gpios[i].new EDGE gpios[i].prev)
				gpios[i].count++;
			gpios[i].prev = gpios[i].new;
		}
		xSemaphoreGive(gpios_lock);
		vTaskDelay(CNT_DEL);
	}
}
