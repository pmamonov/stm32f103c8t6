#ifndef __adc_h
#define __adc_h

#include "stm32f10x_gpio.h"
#include "FreeRTOS.h"
//#include "task.h"
#include "semphr.h"

#define ADC_NCHAN 64
#define ADC_MUX_NCHAN 8
#define ADC_MUX_NPINS 4

#define ADC_MUX_RCC RCC_APB2Periph_GPIOC
#define ADC_MUX_GPIO GPIOC
#define ADC_MUX_PINS {GPIO_Pin_6,GPIO_Pin_7,GPIO_Pin_8,GPIO_Pin_9}

//void adc_init(void);

//int adc_get(int i);

void vADCTask(void* vpars);

extern int adc_vals[ADC_NCHAN];
extern xSemaphoreHandle adc_vals_lock;

#endif
