#/usr/bin/python

from numpy import loadtxt
import matplotlib.pyplot as plt
from numpy import polyfit, polyval

def do_plot(tv, lab):
	t = (tv[:,0] - tv[0,0]) / 60. / 60
	r = polyfit(t, tv[:, 1], 1)
	v = tv[:,1] - r[1]
	l = "%s, %d min, %d mv/day" % (lab, 60 * t[1], -24  * r[0])
	plt.plot(t, v, '-', label=l)
	t2 = (0, 24)
	plt.plot(t2, polyval(r, t2) - r[1], "--k")

b = loadtxt("bat.log");
ex = (b[:314,:], b[350:1700,:], b[1702:2019,:], b[2019:2167,:], b[2167:,:])
ls = ("always on", "always on", "tickless idle", "tickless idle", "tickless idle")

for i in range(len(ex)):
	do_plot(ex[i], ls[i])

plt.xlim(0, 24)
plt.xticks(range(0, 25, 2))
plt.ylim(-255, 25)
plt.xlabel("time, h")
plt.ylabel("discharge, mV")
plt.legend()
plt.grid()
plt.title("""18650 discharge rate VS sleep mode & polling period
Blue Pill @ 8 MHz + Seeed Studio Bluetooth Shield""")
plt.show()
