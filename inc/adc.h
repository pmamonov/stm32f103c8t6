#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f10x_gpio.h"
#include "FreeRTOS.h"
//#include "task.h"
#include "semphr.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CHAN	8
#define CHANF	(1 << CHAN)
#define PERIOD	100
#define DPERIOD	500
#define ADC_NCAL	10

void adc_init(int);
int adc_get(int);
int adc_get_stored(int);
void vADCTask(void* vpars);
int adc_cal_set_xy(int, unsigned long, unsigned long);
int adc_cal_get_xy(int, unsigned long *, unsigned long *);
int adc_cal_save();

#endif /* __ADC_H__ */
