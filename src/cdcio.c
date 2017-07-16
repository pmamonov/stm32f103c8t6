#include "string.h"
#include "cdcio.h"

volatile char	cdc_in_buf[IOBUFLEN];
volatile char	cdc_out_buf[IOBUFLEN];
cdc_buf_t	cdc_in = {cdc_in_buf, 0, 0};
cdc_buf_t	cdc_out = {cdc_out_buf, 0, 0};

uint16_t cdc_write_buf(cdc_buf_t *buf, char *s, uint16_t len, unsigned char block)
{
	uint16_t i = 0;

	if (len == 0)
		len = strlen(s);

	do {
		while (i < len && ((buf->in + 1) % IOBUFLEN) != buf->out) {
			buf->buf[buf->in] = s[i++];
			buf->in = (buf->in + 1) % IOBUFLEN;
		}
	} while (block && i < len);

	return i;
}

uint16_t cdc_read_buf(cdc_buf_t *buf, char *s, uint16_t len)
{
	uint16_t i = 0;

	while (i < len && buf->out != buf->in) {
		s[i++] = buf->buf[buf->out];
		buf->out = (buf->out + 1) % IOBUFLEN;
	}

	return i;
}

uint16_t cdc_gets(char *s, uint16_t len)
{
	uint16_t i = 0;

	if (len < 2)
		goto out;

	while (i < len - 1) {
		i += cdc_read_buf(&cdc_in, &s[i], 1);
		if (i && (s[i - 1] == '\r' || s[i - 1] == '\n'))
			break;
	}

out:
	s[i] = 0;
	return i;
}
