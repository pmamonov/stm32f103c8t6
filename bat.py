#!/usr/bin/python

import sys
from time import time, sleep, strftime
from serial import Serial

TTY = sys.argv[1]
BAUDRATE = int(sys.argv[2])
LOG = sys.argv[3]

with Serial(TTY, baudrate = BAUDRATE) as s, open(LOG, "a") as f:
	while 1:
		s.write("bat\r\n")
		v = int(s.readline().split()[1])
		f.write("%d %d\n" % (time(), v))
		f.flush()
		print("%s\t%.3f" % (strftime("%Y-%m-%d %H:%M:%S"), 1e-3 * v))
		sleep(120)
