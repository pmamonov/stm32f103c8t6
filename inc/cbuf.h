#ifndef __CBUF_H__
#define __CBUF_H__

struct cbuf {
	volatile char *buf;
	unsigned len;
	volatile unsigned in;
	volatile unsigned out;
};

unsigned cbuf_write(struct cbuf *buf, char *s, unsigned len, int block);
unsigned cbuf_read(struct cbuf *buf, char *s, unsigned len, int block);

#endif /* __CBUF_H__ */
