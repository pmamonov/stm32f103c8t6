import sys
from time import time, sleep
from serial import Serial

tty = "/dev/ttyACM0"

try:
	l = int(sys.argv[1])
	n = int(sys.argv[2])
except:
	print "%s <NUM_BYTES> <NUM_WRITES>" % sys.argv[0]
	exit(1)

s = Serial(tty)

s.write("\recho 0\r")
sleep(1)
s.read_all()

r = []
t = time()
for i in xrange(n):
	s.write("rx %d\r" % l)
	s.readline()

	s.write(l * "\x55")
	r.append(s.readline())

t = time() - t

print "tx: %d B/s" % (l * n / t)

b = sum(map(lambda x: int(x.split()[0]), r))
ms = sum(map(lambda x: int(x.split()[1]), r))
print "rx: %d B, %d (%d) B/s" % (b, b / t, (b * 1000 / ms))
