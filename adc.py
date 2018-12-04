import sys
from time import time, sleep
from serial import Serial
from struct import unpack
import numpy as np 

F = 219512		# hardcoded in firmware
BS = F / 4		# 4 data blocks per second

def adc_unpack(d):
	l = map(lambda i: unpack("<H", d[i:i+2])[0] & 0xfff, range(0, len(d), 2))
	return np.array(l, dtype=np.uint16)

class adc:
	def __init__(self, tty, timeout = 10):
		self.s = Serial(tty, timeout = timeout)
		self.s.write("\recho 0\r")
		sleep(1)
		self.s.read_all()

	def start(self, chan=1, f=3): 
		self.s.write("xadc %x %x\r" % (chan, f))

	def stop(self):
		self.s.write("xadc 0\r")

	def read_raw(self, n):
		return self.s.read(2 * n)

	def read(self, n):
		return adc_unpack(self.read_raw(n))

if __name__ == "__main__":
	tty = sys.argv[1]
	fn = sys.argv[2]
	a = adc(tty)
	a.stop()
	a.start()
	with open(fn, "w") as fd:
		sz = 0
		while 1:
			t = time()
			d = a.read_raw(BS)
			fd.write(d)
			sz += len(d)
			print >> sys.stderr, "\r%8d kB, %8d kB/s" % (
				sz >> 10,
				(len(d) >> 10) / (time() - t)),
