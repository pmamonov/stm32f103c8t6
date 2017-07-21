#ifndef __CDCIO_H__
#define __CDCIO_H__
#include <stdint.h>

#define IOBUFLEN 128

typedef struct cdc_buf {
	volatile char *buf;
	volatile uint16_t in;
	volatile uint16_t out;
} cdc_buf_t;

extern cdc_buf_t cdc_in;
extern cdc_buf_t cdc_out;

uint16_t cdc_write_buf(cdc_buf_t *buf, char *s, uint16_t len, unsigned char block);
uint16_t cdc_read_buf(cdc_buf_t *buf, char *s, uint16_t len);

uint16_t cdc_gets(char *s, uint16_t len);
#endif /* __CDCIO_H__ */
