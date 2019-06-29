#!/bin/python


import sys
from time import time, localtime, strftime, sleep
from serial import Serial

try:
	tty = Serial(sys.argv[1])
	log = open(sys.argv[2], "aw")
except Exception as e:
	print e
	print "USAGE: %s /dev/ttyXXX file.log" % sys.argv[0]
	sys.exit(1)

tty.write("\recho 0\r")
print "I2C bus scan"
tty.write("i2cdetect\r")
while 1:
	r = tty.readline().strip()
	print r.strip()
	if r == "DONE":
		break
while 1:
	tty.write("temp\r")
	tim = time()
	temp = tty.readline().strip().split()[-1]
	s = " ".join((strftime("%Y-%m-%d %H:%M:%S", localtime(tim)),
			str(tim),
			temp))
	print s
	log.write(s + "\n")
	sleep(1)
