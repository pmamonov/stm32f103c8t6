import sys
from time import time, sleep
from serial import Serial
from struct import pack, unpack

tty = "/dev/ttyACM0"
bs=4096

try:
	l = int(sys.argv[1])
	n = int(sys.argv[2])
except:
	print "%s <NUM_BYTES> <NUM_WRITES>" % sys.argv[0]
	exit(1)

s = Serial(tty, timeout = 10000)

s.write("\recho 0\r")
sleep(1)
s.read_all()

s.write("rx %d\r" % bs)
s.readline()

s.write("".join(map(lambda x: pack("<L", x * 4), range(bs / 4))))
s.readline()

t = time()
s.write("tx %d %d\r" % (l, n))
r = s.read(l * n)
t = time() - t

for i in xrange(len(r) / 4):
	x = unpack("<L", r[i * 4 : (i + 1) * 4])[0]
	if x != (i * 4) % bs:
		print "mismatch @ %x: %x" % (i * 4, x)

print s.readline()

print "rx: %d B, %d B/s" % (len(r), len(r) / t)
