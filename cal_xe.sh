#!/bin/bash

TTY='/dev/ttyACM0'

function adc_cal()
{
	printf 'cal %d %d %d\r' $1 $2 $3 > $TTY
	sleep 0.1
}

adc_cal 0 620 0
adc_cal 1 1974 3281
adc_cal 2 2595 6874
adc_cal 3 2855 10312
adc_cal 4 3091 13749
adc_cal 5 4000 13749
adc_cal 6 0 0
adc_cal 7 0 0
adc_cal 8 0 0
adc_cal 9 0 0

# printf 'cal_save\r' > $TTY
