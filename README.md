This software is part of the [ASPAM project](https://github.com/comcon1/ASPAM).

### Установка зависимостей / Install dependencies

```sh
	$ sudo apt-get install make binutils-arm-none-eabi gcc-arm-none-eabi openocd minicom
```

### Клонирование и сборка / Clone & build

```sh
	$ git clone https://github.com/pmamonov/stm32f103c8t6
	$ cd stm32f103c8t6
	$ make -s
```

### Прошивка / Firmware upload

|ST-LINK V2	| STM32 BLUE PILL	|
|---------------|-----------------------|
|GND		| GND			|
|SWCLK		| CLK			|
|SWDIO		| DIO			|

```sh
	$ sudo make load_stlink
```

### Командная строка / CLI access

```sh
	$ sudo minicom -D /dev/ttyACMx # x = 0, 1 ...
	> help
	...
```
