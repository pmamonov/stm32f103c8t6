#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x_flash.h"
#include "adc.h"

#define APP_FLASH	(0x08000000 + (63 << 10))

struct adc_cal_flash {
	unsigned long	flow_cal_x[ADC_NCAL];
	unsigned long	flow_cal_y[ADC_NCAL];
};

struct app_flash_s {
	uint32_t		deadbeef;
	struct adc_cal_flash	adc_cal;
};

extern struct app_flash_s app_flash;

int flash_save();
int flash_load();
int flash_is_valid();

#endif /* __FLASH_H__ */
