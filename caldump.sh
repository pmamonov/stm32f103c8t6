#!/bin/bash

TTY='/dev/ttyACM0'

function adc_cal()
{
	printf '%scal %d\r' "$2" $1 > $TTY
	sleep 0.1
}

cat $TTY &
pcat=$!

for i in $(seq 0 9); do 
	adc_cal $i
done
for i in $(seq 0 9); do 
	adc_cal $i x
done

kill $pcat
wait $pcat
