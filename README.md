| stm32f103c8t6 | что подключать		|
|---------------|-------------------------------|
| A8		| PWM_00			|
| A9		| PWM_01			|
| B12		| SENS_00			|
| B13		| SENS_01			|
| B14		| SENS_02			|
| B15		| SENS_03			|
| B8		| PWM_10			|
| B9		| PWM_11			|
| B6		| SENS_10			|
| B7		| SENS_11			|
| B10		| SENS_12			|
| B11		| SENS_13			|

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
