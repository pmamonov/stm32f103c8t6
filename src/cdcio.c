#define __cdcio_c__
#include "cdcio.h"
#include "string.h"

uint16_t cdc_write_buf(cdc_buf_t *buf, char *s, uint16_t len, unsigned char block){
  uint16_t i=0;
  if (len == 0) len = strlen(s);
  do{
    while (i < len && (buf->in < IOBUFLEN-1 ? buf->in + 1 : 0) != buf->out){
      buf->buf[buf->in++] = s[i++];
      if (buf->in >= IOBUFLEN) buf->in=0;
    }
  }while (i<len && block);
  return i;
}

uint16_t cdc_read_buf(cdc_buf_t *buf, char *s, uint16_t len){
  uint16_t i=0;
  while (i < len && buf->out != buf->in){
    s[i++] = buf->buf[buf->out++];
    if (buf->out >= IOBUFLEN) buf->out=0;
  }
  return i;
}

uint16_t cdc_gets(char *s, uint16_t len){
  uint16_t i=0;
  while (i<len-1  &&  (i ? s[i-1] != '\r' : 1) ){
    i += cdc_read_buf(&cdc_in, &s[i], 1);
  }
  s[i]=0;
}
