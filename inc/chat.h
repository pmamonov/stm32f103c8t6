#ifndef __chat_h
#define __chat_h

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "adc.h"
#include "cdcio.h"
#include "string.h"


struct chat_rw_funcs {
	unsigned (*read)(char *s, unsigned len);
	void (*write)(char *s, unsigned len);
};

void vChatTask(void *vpars);

#endif
