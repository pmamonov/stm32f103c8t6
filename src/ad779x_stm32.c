#include <FreeRTOS.h>
#include <task.h>
#include <ad779x.h>
#include <ad779x_stm32.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>

int spi_err;

static void spi_init(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_4;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpio);

	GPIO_SetBits(GPIOA, GPIO_Pin_4);

	gpio.GPIO_Pin = GPIO_Pin_6;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING ;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpio);

	SPI_StructInit(&spi);
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_CPOL = SPI_CPOL_High;
	spi.SPI_CPHA = SPI_CPHA_2Edge;
  	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler =
		SPI_BaudRatePrescaler_256; /* 4.5 MHz / 256 = 17.5 kHz */

	SPI_Cmd(AD779X_SPI, DISABLE);
	SPI_Init(AD779X_SPI, &spi);
	SPI_SSOutputCmd(AD779X_SPI, ENABLE);
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
	SPI_Cmd(AD779X_SPI, ENABLE);

	spi_err = 0;
}

static int spi_wait_txe()
{
	int t;

	for (t = 10; t && !SPI_I2S_GetFlagStatus(AD779X_SPI, SPI_I2S_FLAG_TXE); t--)
		vTaskDelay(1);

	spi_err = t ? 0 : 1;

	return !t;
}

static int spi_wait_rxne()
{
	int t;

	for (t = 10; t && !SPI_I2S_GetFlagStatus(AD779X_SPI, SPI_I2S_FLAG_RXNE); t--)
		vTaskDelay(1);

	spi_err = t ? 0 : 2;

	return !t;
}

void spi_tx(unsigned char Data)
{
	spi_wait_txe();
	SPI_I2S_SendData(AD779X_SPI, Data);
	SPI_I2S_ClearFlag(AD779X_SPI, SPI_I2S_FLAG_RXNE);
	SPI_I2S_ReceiveData(AD779X_SPI);
	spi_wait_rxne();
}

static unsigned char spi_rx(void)
{
	spi_tx(0);
	return SPI_I2S_ReceiveData(AD779X_SPI);
}

static void spi_cs(unsigned char State)
{
	if (State == cssEnable)
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	else
		GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

tAD779X_Device ADCDevice = {
	.CSControl = &spi_cs,
	.TxByte = &spi_tx,
	.RxByte = &spi_rx,
};

static int wait_ready()
{
	int t;

	for (t = 200; t && !AD779X_CheckReadySW(); t--)
		vTaskDelay(10);

	if (!t)
		return 1;

	return 0;
}

int ad779x_stm32_init()
{
	spi_init();

	AD779X_Reset();
	AD779X_Init();
	if (ADCDevice.SuState == susNoHW)
		return 1;

	AD779X_SetClkSource(cssInt);
	AD779X_SetUpdateRate(fs4_17_74dB);
#if 0
	/* TODO: calibrate all channels */
	AD779X_StartZSCalibration();
	if (wait_ready())
		return 2;

	AD779X_StartFSCalibration();
	if (wait_ready())
		return 3;
#endif
	AD779X_SetMode(mdsIdle);

	return 0;
}

unsigned long ad779x_stm32_read(int chan)
{
	/* TODO: select chan */

	AD779X_SetMode(mdsSingle);
	if (wait_ready())
		return -1;

	return AD779X_ReadDataRegister24();
}
