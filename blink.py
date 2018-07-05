#!/usr/bin/python

import sys, time, serial

port = sys.argv[1]
delay = float(sys.argv[2])

s = serial.Serial(port=port, timeout=delay)

while 1:
	try:
		s.write("blink\r")
		print s.read(4096)
	except Exception as e:
		s.close()
		print e
		time.sleep(1)
		s = serial.Serial(port=port, timeout=delay)
