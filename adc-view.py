import sys
from time import time, sleep
import matplotlib.pyplot as plt
import numpy as np
import adc
from threading import Thread, Lock

NP = 2000
RPS = 10
FPS = 2

run = 1

def kbd_reader():
	global run

	sys.stdin.readline()
	run = 0

def adc_reader(a, data, readlen):
	global run

	while run:
		data.append(a.read_raw(readlen))

tty = sys.argv[1]

try:
	scale = int(sys.argv[2])
except:
	scale = 10000

if NP > scale:
	NP = scale
else:
	scale = NP * (scale / NP)

try:
	fd = open(sys.argv[3], "w")
except:
	fd = None

x = np.arange(NP)
y = 0 * x
plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)
l, = ax.plot(x, y, "b-")
ax.set_ylim(0,0x1000)
ax.grid()

a = adc.adc(tty)
a.stop()
a.start()

D = []

rt = Thread(target = adc_reader, args=(a, D, adc.F / RPS))
rt.start()

kt = Thread(target = kbd_reader)
kt.start()

sz = 0
t = time()
while run:
	if not len(D):
		sleep(1. / RPS)
		continue

	_D = map(adc.adc_unpack, D)
	N = sum(map(len, _D))
	while N - _D[0].size >= scale:
		r = D.pop(0)
		e = _D.pop(0)
		N -= e.size
		sz += len(r)
		if fd:
			fd.write(r)
			print >>sys.stderr, "\r%10d MB" % (sz >> 20),
	d = np.concatenate(_D, axis=0)[0 if N < scale else N - scale:]
	y[:1 + d.size / (scale / y.size)] = d[::scale / y.size]
	l.set_ydata(y)
	fig.canvas.draw()

	t += 1. / FPS
	now = time()
	if now < t:
		sleep(t - now)
	else:
		t = now
if fd:
	for d in D:
		fd.write(d)
	fd.close()

rt.join()
kt.join()
