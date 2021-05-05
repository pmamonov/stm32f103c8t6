#ifndef __UART_H__
#define __UART_H__

int uart_init(int id, unsigned baudrate);

int uart_putc(int id, unsigned char c);
int uart_write(int id, unsigned char *s, int len);
int uart_puts(int id, unsigned char *s);

int uart_getc(int id);
int uart_read(int id, unsigned char *s, int len);
int uart_read0(int id, unsigned char *s, int len);
int uart_readline(int id, unsigned char *s, int len);

#endif /*__UART_H__*/
