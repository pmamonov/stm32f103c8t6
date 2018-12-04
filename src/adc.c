#include "string.h"
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

static volatile int chmask;
static volatile int dbuf_ready;
#define BLEN	2048
uint16_t dbuf[BLEN];

void adc_init(int chans)
{
	int i;
	uint32_t tmp;
	GPIO_InitTypeDef sGPIOinit;
	ADC_InitTypeDef sADCinit;

	chmask = chans & ((1 << ARRAY_SIZE(chan_list)) - 1);

	/* set  adc clock to 72/6=12MHz */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* enable  ADC and input GPIOs clock */
	tmp = RCC_APB2Periph_ADC1;
	for (i = 0; i < ARRAY_SIZE(chan_list); i++)
		if (chans & (1 << i))
			tmp |= chan_list[i].rcc;
	RCC_APB2PeriphClockCmd(tmp, ENABLE);

	ADC_DeInit(ADC1);

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
	if (!(chmask & (1 << i)))
		return -1;

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

void adc_dma_start()
{
	int i, nch = 0;
	ADC_InitTypeDef sADCinit;
	DMA_InitTypeDef dma_cfg = {
		.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR,
		.DMA_MemoryBaseAddr = (uint32_t)dbuf,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_BufferSize = sizeof(dbuf) / 2,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_Mode = DMA_Mode_Circular,
		.DMA_Priority = DMA_Priority_Medium,
		.DMA_M2M = DMA_M2M_Disable,
	};
	NVIC_InitTypeDef nvic = {
		.NVIC_IRQChannel = DMA1_Channel1_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE,
	};

	NVIC_Init(&nvic);

	memset(dbuf, 0xaa, sizeof(dbuf));

	for (i = 0; i < ARRAY_SIZE(chan_list); i++)
		if (chmask & (1 << i))
			nch++;

	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);

	/* set  adc clock to 72 / 8 = 9MHz */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* adc setup */
	ADC_DeInit(ADC1);

	sADCinit.ADC_Mode = ADC_Mode_Independent;
	sADCinit.ADC_ScanConvMode = ENABLE;
	sADCinit.ADC_ContinuousConvMode = ENABLE;
	sADCinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	sADCinit.ADC_DataAlign = ADC_DataAlign_Right;
	sADCinit.ADC_NbrOfChannel = nch;
	ADC_Init(ADC1,&sADCinit);

	ADC_Cmd(ADC1, ENABLE);

	for (i = 0; i < ARRAY_SIZE(chan_list); i++)
		if (chmask & (1 << i))
			/* 9 MHz / (12.5 + 28.5) = 219512 Hz */
			ADC_RegularChannelConfig(ADC1, i, 1, ADC_SampleTime_28Cycles5);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_Init(DMA1_Channel1, &dma_cfg);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE);

	ADC_DMACmd(ADC1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1)) {
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		dbuf_ready = 1;
	}
	DMA_ClearITPendingBit(DMA1_IT_GL1);
}

int adc_db_sz()
{
	return sizeof(dbuf);
}

int adc_dma_bytes_ready()
{
	return sizeof(dbuf) - 2 * DMA_GetCurrDataCounter(DMA1_Channel1);
}

void *adc_dma_buf()
{
	return dbuf;
}

void adc_dma_ready_clr()
{
	dbuf_ready = 0;
}

int adc_dma_ready()
{
	return dbuf_ready;
}
