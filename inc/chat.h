#ifndef __chat_h
#define __chat_h

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "adc.h"
#include "cdcio.h"
#include "string.h"

void vChatTask(void *vpars);

#endif
