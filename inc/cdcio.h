#ifndef __cdcio_h__
#define __cdcio_h__
#include <stdint.h>

#define IOBUFLEN 128
typedef struct cdc_buf {
  volatile char *buf;
  volatile uint16_t in;
  volatile uint16_t out;
} cdc_buf_t;

#ifdef __cdcio_c__
volatile char cdc_in_buf[IOBUFLEN];
volatile char cdc_out_buf[IOBUFLEN];
cdc_buf_t cdc_in = {cdc_in_buf, 0, 0};
cdc_buf_t cdc_out = {cdc_out_buf, 0, 0};
#else
extern cdc_buf_t cdc_in;
extern cdc_buf_t cdc_out;
#endif

uint16_t cdc_write_buf(cdc_buf_t *buf, char *s, uint16_t len, unsigned char block);
uint16_t cdc_read_buf(cdc_buf_t *buf, char *s, uint16_t len);

uint16_t cdc_gets(char *s, uint16_t len);
#endif
