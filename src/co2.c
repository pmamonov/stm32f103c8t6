#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "lcd.h"
#include "co2.h"

#define CMD_LEN		9

#define CO2_LINE	2
#define CO2_COL		0

unsigned char cmd_read[CMD_LEN] = {0xff, 1, 0x86, 0, 0, 0, 0, 0, 0x79};
unsigned char cmd_cal_zp[CMD_LEN] = {0xff, 1, 0x87, 0, 0, 0, 0, 0, 0x78};
unsigned char cmd_cal_sp[CMD_LEN] = {0xff, 1, 0x88, 7, 0xd0, 0, 0, 0, 0xa0};

static char csum(char *packet)
{
	char i, checksum = 0;

	for (i = 1; i < 8; i++)
		checksum += packet[i];

	checksum = 0xff - checksum;
	checksum += 1;

	return checksum;
}

static int co2_cmd(char *cmd, char *reply)
{
	int i;
	portTickType timeout = xTaskGetTickCount() + 1000;

	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
		USART_ReceiveData(USART1);

	for (i = 0; i < CMD_LEN; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET)
			;
		USART_SendData(USART1, cmd[i]);
	}

	if (!reply)
		return 0;

	/* FIXME: use interrupt to receive chars */
	for (i = 0; i < CMD_LEN; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != SET)
			if (xTaskGetTickCount() > timeout)
				return 1;
		reply[i] = USART_ReceiveData(USART1);
	}

	if (reply[0] != 0xff ||
	    reply[1] != cmd[2] ||
	    csum(reply) != reply[CMD_LEN - 1])
		return 1;

	return 0;
}

void co2_task(void *vpars)
{
	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef usart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* TX */
	gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init.GPIO_Pin = GPIO_Pin_9;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);
	/* RX */
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_init.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &gpio_init);

	/* USART configuration */
	usart_init.USART_BaudRate = 9600;
	usart_init.USART_WordLength = USART_WordLength_8b;
	usart_init.USART_StopBits = USART_StopBits_1;
	usart_init.USART_Parity = USART_Parity_No;
	usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &usart_init);
	USART_Cmd(USART1, ENABLE);

	while (1) {
		int i, val;
		unsigned char reply[CMD_LEN];
		char s[21];

		if (co2_cmd(cmd_read, reply)) {
			lcd_setstr(CO2_LINE, CO2_COL, "CO2:   N/A");
			continue;
		}
		val = ((int)reply[2] << 8) | reply[3];
		sniprintf(s, sizeof(s), "CO2:%6d", val);
		lcd_setstr(CO2_LINE, CO2_COL, s);

#if 0
		for (i = 0; i < CMD_LEN; i++)
			sniprintf(s + 2 * i, 3, "%02x", reply[i]);
		lcd_setstr(3, 0, s);
#endif

		vTaskDelay(CO2_DISP_PERIOD);
	}
}


