#!/bin/bash

TTY='/dev/ttyACM0'

function adc_cal()
{
	printf 'cal %d %d %d\r' $1 $2 $3 > $TTY
	sleep 0.1
}

adc_cal 0 620 30
adc_cal 1 2048 4167
adc_cal 2 2638 8333
adc_cal 3 2948 12500
adc_cal 4 3103 16667
adc_cal 5 0 0
adc_cal 6 0 0
adc_cal 7 0 0
adc_cal 8 0 0
adc_cal 9 0 0

# printf 'cal_save\r' > $TTY
