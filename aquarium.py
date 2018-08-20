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
    self.s.write("adc_init 0 5\r")
    self.s.read(1024)

  # i equals the number of ADC channel: 0 -- 1, ... 5 -- 
  def adc(self, i):
    while 1:
      self.s.write("adc %d 0\r" % i)
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

  # i equals the number of the STM output pin: 0 -- B10, 1 -- B11, 2 -- B12, 3 -- B13, 4 -- B14, and 5 -- B15
  # v is a signal, 1 -- swith on, 0 -- switch off
  # timeout is a switch-on time interval in ms
  def gpio(self, i, v, timeout=1000):
    self.s.write("gpio %d %d %d\r" % (i, v, timeout))
    self.s.readline()
    self.s.readline()

