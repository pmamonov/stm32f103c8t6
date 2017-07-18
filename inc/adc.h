#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f10x_gpio.h"
#include "FreeRTOS.h"
//#include "task.h"
#include "semphr.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void adc_init(int);
int adc_get(int);
int adc_get_stored(int);
void vADCTask(void* vpars);

#endif /* __ADC_H__ */
