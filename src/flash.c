#include "flash.h"
#include "string.h"
#include "FreeRTOS.h"
#include "semphr.h"

struct app_flash_s app_flash;

xSemaphoreHandle flash_lock = NULL;

int flash_save()
{
	int i;

	xSemaphoreTake(flash_lock, portMAX_DELAY);
	app_flash.deadbeef = 0xdeadbeef;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP |
			FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(APP_FLASH);
	for (i = 0; i < sizeof(app_flash); i += sizeof(uint32_t))
		FLASH_ProgramWord(APP_FLASH + i,
				  *((uint32_t *)((void *)&app_flash + i)));
	FLASH_Lock();
	xSemaphoreGive(flash_lock);

	return 0;
}

int flash_load()
{
	flash_lock = xSemaphoreCreateMutex();

	if (!flash_is_valid())
		return 1;

	memcpy(&app_flash, (void *)APP_FLASH, sizeof(app_flash));

	return 0;
}

int flash_is_valid()
{
	return *(uint32_t *)APP_FLASH == 0xdeadbeef;
}
