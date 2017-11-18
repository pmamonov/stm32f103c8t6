#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"

enum i2c_err {
    EBUSY = 1,
    EMASTER,
    ETRANS,
    ERECV,
    EMBTRANS,
};

void init_I2C1(void);
int I2C_StartTransmission(I2C_TypeDef* I2Cx,
			   uint8_t transmissionDirection,
			   uint8_t slaveAddress);
int I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data);
#endif /*__I2C_H__*/
