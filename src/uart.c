#include <string.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <cbuf.h>
#include <uart.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define RX_BUF_LEN	64

struct uart {
	GPIO_TypeDef	*tx_gpio;
	uint16_t	tx_gpio_pin;
	uint32_t	tx_gpio_rcc;
	void (*tx_gpio_rcc_init)(uint32_t, FunctionalState);

	uint32_t	afio_rcc;
	void (*afio_rcc_init)(uint32_t, FunctionalState);

	GPIO_TypeDef	*rx_gpio;
	uint16_t	rx_gpio_pin;
	uint32_t	rx_gpio_rcc;
	void (*rx_gpio_rcc_init)(uint32_t, FunctionalState);

	USART_TypeDef	*uart;
	uint32_t	uart_rcc;
	void (*uart_rcc_init)(uint32_t, FunctionalState);
	IRQn_Type	irq;

	struct cbuf	rx_buf;
};

static char uart0_rx_buf[RX_BUF_LEN];
static char uart1_rx_buf[RX_BUF_LEN];
static char uart2_rx_buf[RX_BUF_LEN];

static struct uart uart_list[] = {
	[0] = {
		.uart			= USART1,
		.uart_rcc		= RCC_APB2Periph_USART1,
		.uart_rcc_init		= RCC_APB2PeriphClockCmd,
		.irq			= USART1_IRQn,

		.afio_rcc		= RCC_APB2Periph_AFIO,
		.afio_rcc_init		= RCC_APB2PeriphClockCmd,

		.tx_gpio		= GPIOA,
		.tx_gpio_pin		= GPIO_Pin_9,
		.tx_gpio_rcc		= RCC_APB2Periph_GPIOA,
		.tx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_gpio		= GPIOA,
		.rx_gpio_pin		= GPIO_Pin_10,
		.rx_gpio_rcc		= RCC_APB2Periph_GPIOA,
		.rx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_buf	= {
			.buf = uart0_rx_buf,
			.len = sizeof(uart0_rx_buf),
		}
	},
	[1] = {
		.uart			= USART2,
		.uart_rcc		= RCC_APB1Periph_USART2,
		.uart_rcc_init		= RCC_APB1PeriphClockCmd,
		.irq			= USART2_IRQn,

		.tx_gpio		= GPIOA,
		.tx_gpio_pin		= GPIO_Pin_2,
		.tx_gpio_rcc		= RCC_APB2Periph_GPIOA,
		.tx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_gpio		= GPIOA,
		.rx_gpio_pin		= GPIO_Pin_3,
		.rx_gpio_rcc		= RCC_APB2Periph_GPIOA,
		.rx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_buf	= {
			.buf = uart1_rx_buf,
			.len = sizeof(uart1_rx_buf),
		}
	},
	[2] = {
		.uart			= USART3,
		.uart_rcc		= RCC_APB1Periph_USART3,
		.uart_rcc_init		= RCC_APB1PeriphClockCmd,
		.irq			= USART3_IRQn,

		.tx_gpio		= GPIOB,
		.tx_gpio_pin		= GPIO_Pin_10,
		.tx_gpio_rcc		= RCC_APB2Periph_GPIOB,
		.tx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_gpio		= GPIOB,
		.rx_gpio_pin		= GPIO_Pin_11,
		.rx_gpio_rcc		= RCC_APB2Periph_GPIOB,
		.rx_gpio_rcc_init	= RCC_APB2PeriphClockCmd,

		.rx_buf	= {
			.buf = uart2_rx_buf,
			.len = sizeof(uart2_rx_buf),
		}
	},
};

int uart_init(int id, unsigned baudrate)
{
	struct uart *uart;
	USART_InitTypeDef us;
	GPIO_InitTypeDef gs;
	NVIC_InitTypeDef is;

	if (id >= ARRAY_SIZE(uart_list))
		return -1;

	uart = &uart_list[id];

	is.NVIC_IRQChannel = uart->irq;
	is.NVIC_IRQChannelPreemptionPriority = 1;
	is.NVIC_IRQChannelSubPriority = 0;
	is.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&is);

	uart->tx_gpio_rcc_init(uart->tx_gpio_rcc, ENABLE);
	uart->rx_gpio_rcc_init(uart->rx_gpio_rcc, ENABLE);
	if (uart->afio_rcc_init && uart->afio_rcc)
		uart->afio_rcc_init(uart->afio_rcc, ENABLE);
	uart->uart_rcc_init(uart->uart_rcc, ENABLE);

	gs.GPIO_Pin = uart->tx_gpio_pin;
	gs.GPIO_Mode = GPIO_Mode_AF_PP;
	gs.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(uart->tx_gpio, &gs);

	gs.GPIO_Pin = uart->rx_gpio_pin;
	gs.GPIO_Mode = GPIO_Mode_IPU;
	gs.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(uart->rx_gpio, &gs);

	us.USART_Mode			= USART_Mode_Rx|USART_Mode_Tx;
	us.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	us.USART_BaudRate		= baudrate;
	us.USART_WordLength		= USART_WordLength_8b;
	us.USART_Parity			= USART_Parity_No;
	us.USART_StopBits		= USART_StopBits_1;
	USART_DeInit(uart->uart);
	USART_Init(uart->uart, &us);
	USART_ITConfig(uart->uart, USART_IT_RXNE, ENABLE);
	USART_Cmd(uart->uart, ENABLE);

	is.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&is);

	return 0;
}

int uart_putc(int id, unsigned char c)
{
	struct uart *uart;

	if (id >= ARRAY_SIZE(uart_list))
		return -1;
	uart = &uart_list[id];

	while (USART_GetFlagStatus(uart->uart, USART_FLAG_TXE) != SET)
		;
	USART_SendData(uart->uart, c);
	return c;
}

int uart_write(int id, unsigned char *s, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		int ret = uart_putc(id, s[i]);

		if (ret < 0)
			return ret;
	}

	return i;
}

int uart_puts(int id, unsigned char *s)
{
	return uart_write(id, s, strlen(s));
}

int uart_getc(int id)
{
	struct uart *uart;
	char c;

	if (id >= ARRAY_SIZE(uart_list))
		return -1;
	uart = &uart_list[id];

	cbuf_read(&uart->rx_buf, &c, 1, 1);
	return c;
}

static int __uart_read(int id, unsigned char *s, int len, int zero, int newline)
{
	int i;

	for (i = 0; i + zero < len; i++) {
		int ret = uart_getc(id);

		if (ret < 0)
			return ret;
		s[i] = ret;
		if (newline && ret == '\n') {
			i += 1;
			break;
		}

	}

	if (zero)
		s[i] = 0;

	return i;
}

int uart_read(int id, unsigned char *s, int len)
{
	return __uart_read(id, s, len, 0, 0);
}

int uart_read0(int id, unsigned char *s, int len)
{
	return __uart_read(id, s, len, 1, 0);
}

int uart_readline(int id, unsigned char *s, int len)
{
	return __uart_read(id, s, len, 1, 1);
}

int uart_flush(int id)
{
	struct uart *uart;
	if (id >= ARRAY_SIZE(uart_list))
		return -1;
	uart = &uart_list[id];
	USART_ITConfig(uart->uart, USART_IT_RXNE, DISABLE);
	USART_ReceiveData(uart->uart);
	uart->rx_buf.in = 0;
	uart->rx_buf.out = 0;
	USART_ITConfig(uart->uart, USART_IT_RXNE, ENABLE);
}

static inline void uart_receive(int id)
{
	struct uart *uart;
	char c;

	uart = &uart_list[id];
	c = USART_ReceiveData(uart->uart);
	cbuf_write(&uart->rx_buf, &c, 1, 0);
}

void USART1_IRQHandler()
{
	uart_receive(0);
}

void USART2_IRQHandler()
{
	uart_receive(1);
}

void USART3_IRQHandler()
{
	uart_receive(2);
}
