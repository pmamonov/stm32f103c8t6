#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "lcd.h"
#include "flash.h"

struct adc_chan {
	uint32_t	rcc;
	GPIO_TypeDef 	*gpio;
	uint16_t	pin;
};

static struct adc_chan chan_list[] = {
	{	/* channel 0 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_0,
	},
	{	/* channel 1 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_1,
	},
	{	/* channel 2 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_2,
	},
	{	/* channel 3 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_3,
	},
	{	/* channel 4 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_4,
	},
	{	/* channel 5 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_5,
	},
	{	/* channel 6 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_6,
	},
	{	/* channel 7 */
		.rcc = 	RCC_APB2Periph_GPIOA,
		.gpio =	GPIOA,
		.pin =	GPIO_Pin_7,
	},
	{	/* channel 8 */
		.rcc = 	RCC_APB2Periph_GPIOB,
		.gpio =	GPIOB,
		.pin =	GPIO_Pin_0,
	},
	{	/* channel 9 */
		.rcc = 	RCC_APB2Periph_GPIOB,
		.gpio =	GPIOB,
		.pin =	GPIO_Pin_1,
	},
	{	/* channel 10 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_0,
	},
	{	/* channel 11 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_1,
	},
	{	/* channel 12 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_2,
	},
	{	/* channel 13 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_3,
	},
	{	/* channel 14 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_4,
	},
	{	/* channel 15 */
		.rcc = 	RCC_APB2Periph_GPIOC,
		.gpio =	GPIOC,
		.pin =	GPIO_Pin_5,
	},
};

int adc_vals[ARRAY_SIZE(chan_list)];

xSemaphoreHandle adc_vals_lock = NULL;

static volatile enum flow flow_type;

void adc_set_flow_type(enum flow ft)
{
	flow_type = ft;
}

void adc_init(int chans)
{
	int i;
	uint32_t tmp;
	GPIO_InitTypeDef sGPIOinit;
	ADC_InitTypeDef sADCinit;

	/* set  adc clock to 72/6=12MHz */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* enable  ADC and input GPIOs clock */
	tmp = RCC_APB2Periph_ADC1;
	for (i = 0; i < ARRAY_SIZE(chan_list); i++)
		if (chans & (1 << i))
			tmp |= chan_list[i].rcc;
	RCC_APB2PeriphClockCmd(tmp, ENABLE);

	/* configure ADC inputs */
	sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
	sGPIOinit.GPIO_Mode = GPIO_Mode_AIN;
	for (i = 0; i < ARRAY_SIZE(chan_list); i++)
		if (chans & (1 << i)) {
			sGPIOinit.GPIO_Pin = chan_list[i].pin;
			GPIO_Init(chan_list[i].gpio, &sGPIOinit);
		}

	/* adc setup */
	sADCinit.ADC_Mode = ADC_Mode_Independent;
	sADCinit.ADC_ScanConvMode = DISABLE;
	sADCinit.ADC_ContinuousConvMode = DISABLE;
	sADCinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	sADCinit.ADC_DataAlign = ADC_DataAlign_Right;
	sADCinit.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1,&sADCinit);

	ADC_Cmd(ADC1,ENABLE);

	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) != SET)
		;
}

int adc_get(int i)
{
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_RegularChannelConfig(ADC1, i, 1, ADC_SampleTime_71Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
		;
	return ADC_GetConversionValue(ADC1);
}

int adc_get_stored(int i)
{
	if (i >= ARRAY_SIZE(chan_list))
		return 0;

	return adc_vals[i];
};

/* ADC units */
static unsigned long flow_cal_x[ADC_NCAL] = {620, 1974, 2595, 2855, 3091, 4000};
/* ul / sec */
static unsigned long flow_cal_y[ADC_NCAL] = {0, 4375, 9166, 13750, 18333, 18333};
/* ADC units */
static unsigned long xflow_cal_x[ADC_NCAL] = {620, 1974, 2595, 2855, 3091, 4000};
/* ul / sec */
static unsigned long xflow_cal_y[ADC_NCAL] = {0, 3281, 6874, 10312, 13749, 13749};

int adc_cal_save()
{
	memcpy(app_flash.adc_cal.flow_cal_x, flow_cal_x, sizeof(flow_cal_x));
	memcpy(app_flash.adc_cal.flow_cal_y, flow_cal_y, sizeof(flow_cal_y));
	memcpy(app_flash.adc_cal.xflow_cal_x, xflow_cal_x, sizeof(xflow_cal_x));
	memcpy(app_flash.adc_cal.xflow_cal_y, xflow_cal_y, sizeof(xflow_cal_y));

	flash_save();
}

static void adc_cal_load()
{
	if (!flash_is_valid())
		return;
	memcpy(flow_cal_x, app_flash.adc_cal.flow_cal_x, sizeof(flow_cal_x));
	memcpy(flow_cal_y, app_flash.adc_cal.flow_cal_y, sizeof(flow_cal_y));
	memcpy(xflow_cal_x, app_flash.adc_cal.xflow_cal_x, sizeof(xflow_cal_x));
	memcpy(xflow_cal_y, app_flash.adc_cal.xflow_cal_y, sizeof(xflow_cal_y));
}

static void get_fcxy(int ft, unsigned long **fcx, unsigned long **fcy)
{
	*fcx = ft ? xflow_cal_x : flow_cal_x;
	*fcy = ft ? xflow_cal_y : flow_cal_y;
}

int adc_cal_set_xy(int ft, int i, unsigned long x, unsigned long y)
{
	unsigned long *fcx, *fcy;

	get_fcxy(ft, &fcx, &fcy);

	if (i >= ADC_NCAL)
		return 1;

	fcx[i] = x;
	fcy[i] = y;

	return 0;
}

int adc_cal_get_xy(int ft, int i, unsigned long *x, unsigned long *y)
{
	unsigned long *fcx, *fcy;

	get_fcxy(ft, &fcx, &fcy);

	if (i >= ADC_NCAL)
		return 1;

	*x = fcx[i];
	*y = fcy[i];

	return 0;
}

static double adc_to_flow(unsigned long adc, int ft)
{
	int i;
	double f;
	unsigned long *fcx, *fcy;

	get_fcxy(ft, &fcx, &fcy);

	if (adc < fcx[0])
		return 0;

	for (i = 0; i < ARRAY_SIZE(flow_cal_x) - 1; i++)
		if (fcx[i] <= adc && adc <= fcx[i + 1])
			break;
	if (i == ARRAY_SIZE(flow_cal_x) - 1)
		return 0;
	f = fcy[i + 1] - fcy[i];
	f = (f * (adc - fcx[i])) / (fcx[i + 1] - fcx[i]);
	f += fcy[i];
	return f * 1e-3; /* ml/s */
}

void vADCTask(void* vpars)
{
	unsigned int adc, mv;
	double f = 0, f0 = 0, v = 0, xv = 0, dt = 1.0 * PERIOD / configTICK_RATE_HZ;
	char s[21];
	portTickType lwt, ldt = 0, t;

	adc_init(CHANF);
	memset(adc_vals, 0, sizeof(adc_vals));
	adc_cal_load();

	lwt = xTaskGetTickCount();

	while (1) {
		enum flow ft = flow_type;

		vTaskDelayUntil(&lwt, PERIOD);
		adc = adc_get(CHAN);
		mv = adc * 3300 / 4096;
		f0 = f;
		switch (ft) {
		case FLOW_AIR:
			f = adc_to_flow(adc, 0);
			v += dt * (f + f0) / 2;
			break;
		case FLOW_XEN:
			f = adc_to_flow(adc, 1);
			xv += dt * (f + f0) / 2;
			break;
		case FLOW_RESET:
			f = v = xv = 0;
			break;
		default:
			break;
		}

		t = xTaskGetTickCount();
		if (t < ldt + DPERIOD)
			continue;
		ldt = xTaskGetTickCount();

		sniprintf(s, sizeof(s), "FLOW: %4d ml/min", (long)(f * 60));
		lcd_setstr(0, 0, s);

		sniprintf(s, sizeof(s), "XEN/AIR:%4d/%4d ml",
					(long)xv, (long)v);
		lcd_setstr(1, 0, s);
	}
}

