#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "pwm.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define NPWM	ARRAY_SIZE(pwm_list)

extern uint32_t SystemCoreClock;

struct pwm {
	unsigned long	hz;
	unsigned long	period;

	GPIO_TypeDef	*gpio;
	uint16_t	gpio_pin;
	uint32_t	gpio_rcc;
	void (*gpio_rcc_init)(uint32_t, FunctionalState);

	uint32_t	afio_rcc;
	void (*afio_rcc_init)(uint32_t, FunctionalState);

	TIM_TypeDef *tim;
	uint32_t	tim_rcc;
	void (*tim_rcc_init)(uint32_t, FunctionalState);

	void (*init)(TIM_TypeDef *, TIM_OCInitTypeDef *);
};

static struct pwm pwm_list[] = {
/*	[0] = {
		.gpio = GPIOA,
		.gpio_pin = GPIO_Pin_8,
		.gpio_rcc = RCC_APB2Periph_GPIOA,
		.gpio_rcc_init = RCC_APB2PeriphClockCmd,

		.afio_rcc = RCC_APB2Periph_AFIO,
		.afio_rcc_init = RCC_APB2PeriphClockCmd,

		.tim = TIM1,
		.tim_rcc = RCC_APB2Periph_TIM1,
		.tim_rcc_init = RCC_APB2PeriphClockCmd,
		.init = TIM_OC1Init,
	},
	[1] = {
		.gpio = GPIOA,
		.gpio_pin = GPIO_Pin_9,
		.gpio_rcc = RCC_APB2Periph_GPIOA,
		.gpio_rcc_init = RCC_APB2PeriphClockCmd,

		.afio_rcc = RCC_APB2Periph_AFIO,
		.afio_rcc_init = RCC_APB2PeriphClockCmd,

		.tim = TIM1,
		.tim_rcc = RCC_APB2Periph_TIM1,
		.tim_rcc_init = RCC_APB2PeriphClockCmd,
		.init = TIM_OC2Init,
	}, */
};

int pwm_count()
{
	return NPWM;
}

int pwm_init(int id, unsigned int hz)
{
	struct pwm *pwm;
	TIM_TimeBaseInitTypeDef ts;
	GPIO_InitTypeDef gs;

	if (id >= NPWM || hz > SystemCoreClock)
		return 1;

	pwm = &pwm_list[id];
	pwm->hz = hz;
	pwm->period = SystemCoreClock / hz - 1;

	if (pwm->period > 0xffff)
		return 1;

	pwm->tim_rcc_init(pwm->tim_rcc, ENABLE);
	pwm->gpio_rcc_init(pwm->gpio_rcc, ENABLE);
	pwm->afio_rcc_init(pwm->afio_rcc, ENABLE);

	gs.GPIO_Pin = pwm->gpio_pin;
	gs.GPIO_Mode = GPIO_Mode_AF_PP;
	gs.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(pwm->gpio, &gs);

	ts.TIM_Prescaler = 0;
	ts.TIM_CounterMode = TIM_CounterMode_Up;
	ts.TIM_Period = pwm->period;
	ts.TIM_ClockDivision = 0;
	ts.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(pwm->tim, &ts);

	pwm_set(id, 0);

	return 0;
}

int pwm_init_all(unsigned int hz)
{
	int i;

	for (i = 0; i < NPWM; i++)
		pwm_init(i, hz);
}

int pwm_set(int id, unsigned int dc)
{
	struct pwm *pwm;
	TIM_OCInitTypeDef  ocs;

	if (id >= NPWM || dc > 100)
		return 1;

	pwm = &pwm_list[id];

	ocs.TIM_OCMode = TIM_OCMode_PWM2;
	ocs.TIM_OutputState = TIM_OutputState_Enable;
	ocs.TIM_OutputNState = TIM_OutputNState_Enable;
	ocs.TIM_Pulse = pwm->period * dc / 100;
	ocs.TIM_OCPolarity = TIM_OCPolarity_Low;
	ocs.TIM_OCNPolarity = TIM_OCNPolarity_High;
	ocs.TIM_OCIdleState = TIM_OCIdleState_Set;
	ocs.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	pwm->init(pwm->tim, &ocs);

	TIM_Cmd(pwm->tim, ENABLE);
	TIM_CtrlPWMOutputs(pwm->tim, ENABLE);

	return 0;
}
