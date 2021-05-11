#include <string.h>
#include <FreeRTOS.h>
#include <cbuf.h>

unsigned cbuf_write(struct cbuf *buf, char *s, unsigned len, int block)
{
	unsigned i = 0;

	if (len == 0)
		len = strlen(s);
	do {
		while (i < len && ((buf->in + 1) % buf->len) != buf->out) {
			buf->buf[buf->in] = s[i++];
			buf->in = (buf->in + 1) % buf->len;
		}
		if (block && i < len)
			vTaskDelay(10);
	} while (block && i < len);

	return i;
}

unsigned cbuf_read(struct cbuf *buf, char *s, unsigned len, int block)
{
	unsigned i = 0;

	do {
		while (i < len && buf->out != buf->in) {
			s[i++] = buf->buf[buf->out];
			buf->out = (buf->out + 1) % buf->len;
		}
		if (block && i < len)
			vTaskDelay(10);
	} while (block && i < len);

	return i;
}
