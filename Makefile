PREFIX=arm-none-eabi
AS      = $(PREFIX)-as
CC      = $(PREFIX)-gcc
LD      = $(PREFIX)-ld
OBJCOPY = $(PREFIX)-objcopy

LDSCRIPT = stm32f103c8t6_flash.ld

CFLAGS = -mcpu=cortex-m3 -mthumb -Os -g\
 -I./inc\
 -ICM3/DeviceSupport/ST/STM32F10x\
 -ICM3/CoreSupport\
 -ISTM32_USB-FS-Device_Driver/inc\
 -ISTM32F10x_StdPeriph_Driver/inc\
 -DSTM32F10X_MD\
 -DUSE_STM3210B_EVAL\
 -DUSE_STDPERIPH_DRIVER\
 -IFreeRTOS/Source/include\
 -IFreeRTOS/Source/portable/GCC/ARM_CM3 \
 -Ii2c_lcd

#src/startup_stm32f10x_md.o\

OBJ = src/startup_stm32f10x_md.o \
  src/pwm.o \
  src/blink.o \
  src/adc.o \
  src/chat.o \
  src/flash.o \
  src/main.o \
  src/hw_config.o\
  src/stm32_it.o\
  src/system_stm32f10x.o\
  src/usb_desc.o\
  src/usb_endp.o\
  src/usb_istr.o\
  src/usb_prop.o\
  src/usb_pwr.o\
  src/cdcio.o\
  src/newlib_stubs.o\
  src/strtok.o\
  FreeRTOS/Source/tasks.o\
  FreeRTOS/Source/queue.o\
  FreeRTOS/Source/list.o\
  FreeRTOS/Source/timers.o\
  FreeRTOS/Source/portable/GCC/ARM_CM3/port.o\
  FreeRTOS/Source/portable/MemMang/heap_1.o\
  STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.o\
  STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.o\
  STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.o\
  STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.o \
  STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.o \
  STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.o \
  STM32F10x_StdPeriph_Driver/src/misc.o\
  STM32_USB-FS-Device_Driver/src/usb_regs.o\
  STM32_USB-FS-Device_Driver/src/usb_int.o\
  STM32_USB-FS-Device_Driver/src/usb_mem.o\
  STM32_USB-FS-Device_Driver/src/usb_init.o\
  STM32_USB-FS-Device_Driver/src/usb_core.o\
  STM32_USB-FS-Device_Driver/src/usb_sil.o\
  CM3/CoreSupport/core_cm3.o

.PHONY: inc/version.h
.PHONY: tags

all: main.bin

main.bin: main.elf
	$(OBJCOPY) -O binary $< $@
	$(PREFIX)-size $<

main.elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ -nostartfiles  -Wl,-T$(LDSCRIPT)  $^
#	$(CC) -o $@ -nostartfiles -Wl,-T$(LDSCRIPT) $^

clean:
	rm -f $(OBJ) *.elf *.bin *.ihex

#%.o: %.c
#  $(CC) $(CFLAGS) -c $< -o $@

# GAS emits error when optimization is enabled
CM3/CoreSupport/core_cm3.o: CM3/CoreSupport/core_cm3.c
	$(CC) $(CFLAGS) -O0 -c -o $@ $<

load: main.bin
	openocd -f stm32f103c8t6-devboard.cfg -f fwload.openocd

load_stlink: main.bin
	openocd -f stm32f103c8t6-stlink.cfg -f fwload.openocd

reset:
	openocd -f stm32f103c8t6-devboard.cfg  -c "init; reset run; shutdown"

reset_stlink:
	openocd -f stm32f103c8t6-stlink.cfg  -c "init; reset run; shutdown"

inc/version.h:
	sh -c 'echo "#define __VERSION \"$$(./setlocalversion)\""' > inc/version.h

src/chat.o: inc/version.h

tags:
	ctags -R .
