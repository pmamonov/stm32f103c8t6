#ifndef AD779X_STM32_H
#define AD779X_STM32_H

#include <ad779x.h>

#define AD779X_SPI	SPI1

extern int spi_err;

int ad779x_stm32_init(int, int);
unsigned long ad779x_stm32_read(int chan);
void spi_tx(unsigned char Data);

#endif /*AD779X_STM32_H*/
