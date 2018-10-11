#!/usr/bin/python

maze_state_machine = {
#        Format:
#
#        "STATE_IN/STATE_OUT < STATE_IN/STATE_OUT": "CMD; CMD; ...",
#
#        STATE = "x1d1x2d2_p1p2_R"
#                 ^^^^^^^^ ^^^^ ^
#                 SENSORS  PWM  RFID_READY
#                               optional
#	CMD = "pwmX S" (X=0..3, S=0,1), "rfid", "error", "reset T" (T = seconds)
#
	"/ < reset":			"pwm0 0; pwm1 1; pwm2 0; pwm3 0",
	"0010_01/ < ...0_01/":		"pwm0 1; pwm1 1; rfid",
	"...._11_R/ < ...._.1/":	"pwm0 1; pwm1 0",
	".000_10/ < .001_10/":		"pwm0 1; pwm1 1; reset 60",
}

PWM_WARMUP_MS = 100
PWM_DC = 30

import sys
from argparse import ArgumentParser
from threading import Thread
from serial import Serial
from time import sleep
import re
from time import time

class tty(object):
	s = None

	def __init__(self, dev, baud=115200, timeout=5):
		self.s = Serial(port=dev, baudrate=baud, timeout=timeout)
		self.s.read_all()

	def __del__(self):
		if self.s:
			self.s.close()

class rfid(tty):
	def __init__(self, dev, baud=115200, timeout=5):
		super(rfid, self).__init__(dev, baud, timeout)
		self.s.write(".ES0\r") # Disable auto shutdown
		self.s.readline()
		self.s.write(".RT05\r") # Set read timeout
		self.s.readline()
		self.s.write(".RB1\r") # Enable read beep
		self.s.readline()

	def read(self):
		self.s.write(".r\r")
		self.s.readline()
		return self.s.readline().strip()

class gate(tty):
	def __init__(self, dev, baud=115200, timeout=5, ngpio=8, npwm=4):
		super(gate, self).__init__(dev, baud, timeout)
		self.ngpio = ngpio
		self.gpio_state = tuple([0 for i in range(ngpio)]) 
		self.pwm_state = [0 for i in range(npwm)]

	def pwm(self, i, duty_cycle, warmup_ms=500):
		self.s.write("pwm %d %d %d\r" % (i, warmup_ms, duty_cycle))
		self.pwm_state[i] = 1 if duty_cycle > 0 else 0

	def gpio_poll(self, timeout=0):
		while 1:
			s = self.s.readline()
			if timeout and s == "":
				return self.gpio_state
			ss = s.split()
			if ss[0] != "gpio:":
				continue
			x = int(ss[1], 0x10)
			ret = map(lambda i, x=x: 1 if x & (1 << i) else 0,
					range(self.ngpio))
			self.gpio_state = ret
			return ret

class msm:
	def __init__(self, rules, gate, rfid):
		self.rules = rules
		self.gate = gate
		self.rfid = rfid
		self.state = "reset"
		self.reset = 0
		self.gpio_state = [0 for i in range(8)]
		self.rfid_ready = False
		self.rfid_thread = None
		self.rfid_id = ""
		self.update()

	def rfid_read(self):
		self.rfid_id = ""
		while len(self.rfid_id) == 0:
		    print "RFID: start read"
		    self.rfid_id = self.rfid.read().strip()
		self.rfid_ready = True
		print "RFID: id=%s" % self.rfid_id

	def get_state(self, block):
		if not block:
			self.gate.s.write("gpio\r")
		self.gpio_state = self.gate.gpio_poll(1)
		self.pwm_state = self.gate.pwm_state
		rfid_ready = self.rfid_ready
		if self.rfid_ready:
			self.rfid_ready = False
			if self.rfid_thread:
				self.rfid_thread.join()
				self.rfid_thread = None
				print "RFID: read finished"
		return ("%d%d%d%d" % tuple(self.gpio_state[:4]) +
			"_%d%d" % tuple(self.pwm_state[:2]) +
			("_R" if rfid_ready else "") +
			"/" +
			"%d%d%d%d" % tuple(self.gpio_state[4:]) +
			"_%d%d" % tuple(self.pwm_state[2:]))

	def apply(self, cmd):
		for _c in cmd.split(";"):
			c = _c.strip()
			if c[:3] == "pwm":
				try:
					i = int(c[3])
					dc = int(c.split()[1])
				except:
					print "bad command `%s`" % c
					continue
				self.gate.pwm(i, PWM_DC if dc else 0, PWM_WARMUP_MS if dc else 0)
				print "pwm %d %d" % (i, dc)
			elif c == "rfid":
				if not self.rfid:
					self.rfid_ready = 1
					continue
				if self.rfid_thread:
					continue
				self.rfid_thread = Thread(target=self.rfid_read)
				self.rfid_thread.start()
				print "RFID thread spawned"

			elif c[:5] == "reset":
				try:
					t = int(c.split()[1])
				except:
					print "bad command `%s`" % c
					continue
				self.reset = time() + t
			else:
				print "unknown command `%s`" % c

	def update(self):
		self.pstate = self.state

		if self.reset and time() >= self.reset:
			self.pstate = "reset"
			self.reset = 0

		self.state = self.get_state(self.pstate != "reset")
		while self.pstate != self.state:
			print "%s -> %s" % (self.pstate, self.state)
			for rule in self.rules.keys():
				n, p = rule.replace(" ", "").split("<")
				if re.search(n, self.state) and re.search(p, self.pstate):
					print "applying `%s` : `%s`" % (rule, self.rules[rule])
					self.apply(self.rules[rule])
			self.pstate = self.state
			self.state = self.get_state(0)

if __name__ == "__main__":
	p = ArgumentParser(description="maze control")
	p.add_argument('-g', dest="gate", metavar='/dev/ttyX',
				type=str, help='gate interface', required=1)
	p.add_argument('-r', dest="rfid", metavar='/dev/ttyY',
				type=str, help='RFID interface', required=0)
	p.add_argument('-l', dest="log", metavar='FILE',
				type=str, help='log file', default='maze.log')
	a = p.parse_args()

	g = gate(a.gate)
	r = rfid(a.rfid) if a.rfid else None
	m = msm(maze_state_machine, g, r)

	while 1:
		m.update()
