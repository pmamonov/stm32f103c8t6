#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"

int adc_vals[ADC_NCHAN];

uint16_t adc_mux_pins[] = ADC_MUX_PINS;

xSemaphoreHandle adc_vals_lock = NULL;

void adc_init(void)
{
	int i;

	GPIO_InitTypeDef sGPIOinit;
	ADC_InitTypeDef sADCinit;
	/* set  adc clock to 48/4=12MHz */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);
	/* enable  ADC and input GPIOs clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
			       RCC_APB2Periph_GPIOB |
			       RCC_APB2Periph_GPIOC |
			       RCC_APB2Periph_ADC1,
			       ENABLE);
	/* configure all 16 adc inputs */
	sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
	sGPIOinit.GPIO_Mode = GPIO_Mode_AIN;

	sGPIOinit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | 
			     GPIO_Pin_2 | GPIO_Pin_3 |
			     GPIO_Pin_4 | GPIO_Pin_5 |
			     GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &sGPIOinit);

	sGPIOinit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |
			     GPIO_Pin_2 | GPIO_Pin_3 |
			     GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &sGPIOinit);

	sGPIOinit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &sGPIOinit);
	/* adc setup */
	sADCinit.ADC_Mode = ADC_Mode_Independent;
	sADCinit.ADC_ScanConvMode = DISABLE;
	sADCinit.ADC_ContinuousConvMode = DISABLE;
	sADCinit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	sADCinit.ADC_DataAlign = ADC_DataAlign_Right;
	sADCinit.ADC_NbrOfChannel = 1;

	ADC_Init(ADC1,&sADCinit);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1,
				 ADC_SampleTime_7Cycles5);

	ADC_Cmd(ADC1,ENABLE);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) != SET);

	/* external multiplexer control pins setup */
	RCC_APB2PeriphClockCmd(ADC_MUX_RCC, ENABLE);

	sGPIOinit.GPIO_Speed = GPIO_Speed_10MHz;
	sGPIOinit.GPIO_Mode = GPIO_Mode_Out_OD;
	sGPIOinit.GPIO_Pin = 0;
	for (i = 0; i < ADC_MUX_NPINS; i++)
		sGPIOinit.GPIO_Pin |= adc_mux_pins[i];
	GPIO_Init(ADC_MUX_GPIO, &sGPIOinit);

	return;
}

int adc_get(int i)
{
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_RegularChannelConfig(ADC1, i, 1, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}

void adc_switch_mux(int n)
{
	volatile int i;

	for (i = 0; i < ADC_MUX_NPINS; i++) {
		if ((1 << i) & n)
			GPIO_SetBits(ADC_MUX_GPIO, adc_mux_pins[i]);
		else
			GPIO_ResetBits(ADC_MUX_GPIO, adc_mux_pins[i]);
	}

	for (i = 500; i; i--);
}

void vADCTask(void* vpars)
{
	int i, j, ref;

	adc_init();
	adc_vals_lock = xSemaphoreCreateMutex();

	while (1) {
		xSemaphoreTake(adc_vals_lock, portMAX_DELAY);
		for (i = 0; i < ADC_NCHAN / ADC_MUX_NCHAN; i++) {
			ref = adc_get(ADC_NCHAN / ADC_MUX_NCHAN + i);
			for (j = 0; j < ADC_MUX_NCHAN; j++) {
				adc_switch_mux(j);
				//TODO: delay?
				adc_vals[i*ADC_MUX_NCHAN+j] = adc_get(i)-ref;
			}
		}
		xSemaphoreGive(adc_vals_lock);
		vTaskDelay(100);
	}
}

