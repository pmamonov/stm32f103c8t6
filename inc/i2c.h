#ifndef __I2C_H__
#define __I2C_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "FreeRTOS.h"
#include "task.h"

#define I2C_TIMEOUT	(1 + configTICK_RATE_HZ / 10)

#define I2C_TX	I2C_Direction_Transmitter
#define I2C_RX	I2C_Direction_Receiver

enum i2c_err {
	I2C_ERR_INVAL = 1,
	I2C_ERR_TIMEOUT,
	I2C_ERR_BUSY,
	I2C_ERR_MASTER,
	I2C_ERR_MODE,
	I2C_ERR_XMIT,
	I2C_ERR_RECV,
};

struct i2c_cfg {
	I2C_TypeDef		*i2c;
	GPIO_TypeDef		*gpio;
	uint16_t		gpio_pins;
	volatile uint32_t	*rcc_gpio;
	uint32_t		rcc_gpio_en;
	volatile uint32_t	*rcc_afio;
	uint32_t		rcc_afio_en;
	volatile uint32_t	*rcc_i2c;
	uint32_t		rcc_i2c_en;
};

int i2c_init(unsigned i, unsigned bitrate);
int i2c_start(unsigned i, uint8_t dir,  uint8_t addr);
int i2c_stop(unsigned i);
int i2c_xmit(unsigned i, uint8_t addr, uint8_t *data, unsigned len);
int i2c_recv(unsigned i, uint8_t addr, uint8_t *data, unsigned len);

#endif /* __I2C_H__ */
