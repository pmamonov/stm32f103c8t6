#!/usr/bin/python2.7

import sys
from serial import Serial
from time import sleep

class aquarium:
	s = None

	def __init__(self, tty="/dev/ttyACM0"):
		self.tty = tty
		self.init()

	def init(self):
		if self.s:
			self.s.close()
		self.s = Serial(port=self.tty, timeout=5)
		self.s.write("adc_init\r")
		self.s.read(1024)


	def adc(self, i):
		while 1:
			self.s.write("adc %d\r" % i)
			self.s.readline()
			l = self.s.readline()
			try:
				s = l.split()[1]
				return int(s, 16)
			except ValueError:
				print >>sys.stderr, "W: `%s`" % l.strip()
				continue
			except:
				raise NameError, "`%s`" % l

	def gpio(self, i, v):
		self.s.write("gpio %d %d\r" % (i, v))
		self.s.readline()
		print >>sys.stderr, "I: `%s`" % self.s.readline().strip()

if __name__ == "__main__":
	a = aquarium()
	while 1:
		print "0x%x" % a.adc(0)
		sys.stdout.flush()
		sleep(1)
