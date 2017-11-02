#include "stdio.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_gpio.h"
#include "lcd.h"
#include "adc.h"

#undef DEBUG
#define DEBUG

#define GPIO_BUT_MODE	GPIOA
#define RCC_BUT_MODE	RCC_APB2Periph_GPIOA
#define BUT_IN_PIN	GPIO_Pin_0
#define BUT_CIRC_PIN	GPIO_Pin_1
#define MODE_XIN_PIN	GPIO_Pin_2
#define MODE_AIN_PIN	GPIO_Pin_3
#define MODE_CIRC_PIN	GPIO_Pin_4
#define MODE_VENT_PIN	GPIO_Pin_5

#define GPIO_VALVE_PUMP	GPIOB
#define RCC_VALVE_PUMP	RCC_APB2Periph_GPIOB
#define VALVE_IN_PIN	GPIO_Pin_12
#define PUMP_IN_PIN	GPIO_Pin_13
#define VALVE_CIRC_PIN	GPIO_Pin_14
#define PUMP_CIRC_PIN	GPIO_Pin_15

enum state {
	STATE_OFF = 0,
	STATE_ON,
	STATE_MEDIUM = STATE_ON,
	STATE_FAST,
};

enum mode {
	MODE_XIN = 0,
	MODE_AIN,
	MODE_CIRC,
	MODE_VENT,
	MODE_START,
	MODE_NONE,
};

enum state circ;

static void io_init()
{
	GPIO_InitTypeDef gpio_init;

	/* inputs */
	RCC_APB2PeriphClockCmd(RCC_BUT_MODE, ENABLE);

	gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU;
	gpio_init.GPIO_Pin = BUT_IN_PIN | BUT_CIRC_PIN |
			     MODE_XIN_PIN | MODE_AIN_PIN |
			     MODE_CIRC_PIN | MODE_VENT_PIN;
	GPIO_Init(GPIO_BUT_MODE, &gpio_init);

	/* valves and */
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init.GPIO_Pin = VALVE_IN_PIN | VALVE_CIRC_PIN | 
			     PUMP_IN_PIN | PUMP_CIRC_PIN;
	GPIO_Init(GPIO_VALVE_PUMP, &gpio_init);
}

static enum state valve_in_set(enum state st)
{
#ifdef DEBUG
		lcd_setstr(3, 4, st ? "i:O " : "i:x");
#endif
	GPIO_WriteBit(GPIO_VALVE_PUMP, VALVE_IN_PIN, st);

	return st;
}

static enum state valve_circ_set(enum state st)
{
#ifdef DEBUG
		lcd_setstr(3, 12, st ? "c:O " : "c:x");
#endif
	GPIO_WriteBit(GPIO_VALVE_PUMP, VALVE_CIRC_PIN, st);

	return st;
}

static void pump_set(GPIO_TypeDef* gpio, uint16_t pin, enum state st)
{
	GPIO_InitTypeDef gpio_init;
	BitAction gpio_val = Bit_RESET;
	GPIOMode_TypeDef gpio_mode = GPIO_Mode_IN_FLOATING;

	switch (st) {
	case STATE_OFF:
		gpio_mode = GPIO_Mode_Out_PP;
		gpio_val = Bit_SET;
		break;
	case STATE_MEDIUM:
		gpio_mode = GPIO_Mode_IN_FLOATING;
		break;
	case STATE_FAST:
		gpio_mode = GPIO_Mode_Out_PP;
		gpio_val = Bit_RESET;
		break;
	default:
		break;
	}
	gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
	gpio_init.GPIO_Mode = gpio_mode;
	gpio_init.GPIO_Pin = pin;
	GPIO_Init(gpio, &gpio_init);
	if (gpio_mode == GPIO_Mode_Out_PP)
		GPIO_WriteBit(gpio, pin, gpio_val);
}

static enum state pump_in_set(enum state st)
{
#ifdef DEBUG
		lcd_setstr(3, 8, st == STATE_OFF ? "I:x" :
				 (st == STATE_MEDIUM ? "I:o" : "I:O"));
#endif
	pump_set(GPIO_VALVE_PUMP, PUMP_IN_PIN, st);

	return st;
}

static enum state pump_circ_set(enum state st)
{
#ifdef DEBUG
		lcd_setstr(3, 16, st == STATE_OFF ? "C:x" :
				 (st == STATE_MEDIUM ? "C:o" : "C:O"));
#endif
	pump_set(GPIO_VALVE_PUMP, PUMP_CIRC_PIN, st);

	return st;
}

static enum mode get_mode(void)
{
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, MODE_XIN_PIN) == Bit_RESET)
		return MODE_XIN;
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, MODE_AIN_PIN) == Bit_RESET)
		return MODE_AIN;
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, MODE_CIRC_PIN) == Bit_RESET)
		return MODE_CIRC;
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, MODE_VENT_PIN) == Bit_RESET)
		return MODE_VENT;
	return MODE_NONE;
}

static enum state get_but_in(void)
{
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, BUT_IN_PIN) == Bit_RESET)
		return STATE_ON;

	return STATE_OFF;
}

static enum state get_but_circ(void)
{
	if (GPIO_ReadInputDataBit(GPIO_BUT_MODE, BUT_CIRC_PIN) == Bit_RESET)
		return STATE_ON;

	return STATE_OFF;
}

static enum mode apply_mode(enum mode new, enum mode old)
{
	enum mode ret = new;

	switch (new) {
	case MODE_XIN:
#ifdef DEBUG
		lcd_setstr(3, 0, "M:X ");
#endif
		adc_set_flow_type(FLOW_XEN);
		valve_in_set(STATE_ON);
		pump_in_set(STATE_OFF);
		if (old == MODE_CIRC) {
			valve_circ_set(circ);
			pump_circ_set(circ);
		} else {
			valve_circ_set(STATE_OFF);
			pump_circ_set(STATE_OFF);
			circ = STATE_OFF;
		}
		break;
	case MODE_AIN:
#ifdef DEBUG
		lcd_setstr(3, 0, "M:A ");
#endif
		adc_set_flow_type(FLOW_AIR);
		valve_in_set(STATE_OFF);
		pump_in_set(STATE_OFF);
		if (old == MODE_CIRC) {
			valve_circ_set(circ);
			pump_circ_set(circ);
		} else {
			valve_circ_set(STATE_OFF);
			pump_circ_set(STATE_OFF);
			circ = STATE_OFF;
		}
		break;
	case MODE_CIRC:
#ifdef DEBUG
		lcd_setstr(3, 0, "M:C ");
#endif
		pump_in_set(STATE_OFF);
		pump_circ_set(STATE_OFF);
		valve_in_set(STATE_OFF);
		valve_circ_set(STATE_OFF);
		circ = STATE_OFF;
		break;
	case MODE_VENT:
#ifdef DEBUG
		lcd_setstr(3, 0, "M:V ");
#endif
		adc_set_flow_type(FLOW_RESET);
		valve_in_set(STATE_OFF);
		valve_circ_set(STATE_ON);
		pump_circ_set(STATE_ON);
		pump_in_set(STATE_FAST);
		vTaskDelay(15 * configTICK_RATE_HZ);
		pump_in_set(STATE_OFF);
		circ = STATE_OFF;
		break;
	default:
		ret = old;
		break;
	}

	return ret;
}

void io_task(void *vpars)
{
	enum mode mode = MODE_START, new_mode;
	enum state but_circ_prev = STATE_OFF, but_circ;
	char s[21];

	io_init();

	pump_in_set(STATE_OFF);
	pump_circ_set(STATE_OFF);
	valve_in_set(STATE_OFF);
	valve_circ_set(STATE_OFF);

	circ = STATE_OFF;
	adc_set_flow_type(FLOW_RESET);

	while (1) {
		new_mode = get_mode();
		if (new_mode != mode)
			mode = apply_mode(new_mode, mode);

		if (mode == MODE_AIN) {
			if (get_but_in())
				pump_in_set(STATE_MEDIUM);
			else
				pump_in_set(STATE_OFF);
		}

		but_circ = get_but_circ();
		if (but_circ == STATE_ON &&
		    but_circ_prev == STATE_OFF &&
		    mode != MODE_VENT) {
			circ = !circ;
			valve_circ_set(circ);
			pump_circ_set(circ);
		}
		but_circ_prev = but_circ;

		vTaskDelay(configTICK_RATE_HZ / 10);
	}
}
