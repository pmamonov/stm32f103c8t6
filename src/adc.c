#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"

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

void vADCTask(void* vpars)
{
	int i, j, ref;

	while (1) {
		vTaskDelay(100);
	}
}

