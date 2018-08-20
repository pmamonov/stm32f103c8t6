#!/usr/bin/python

import sys,os
from pylab import *
from time import sleep,time,ctime
from aquarium import *

# function that returns the temperature according to the calibration coefficients stored in 'C'
def tenz2temp(n,U):
  if n == 0:
    C = [3.28562193e+06, 1.07583206e+04, 3.79965530e+00, 2.62228070e+02]
  elif n == 1:
    C = [3.28179616e+06, 1.07015533e+04, 3.78066457e+00, 2.63367037e+02]
  elif n == 2:
    C = [3.29080126e+06, 1.09033110e+04, 3.84716222e+00, 2.59438963e+02]
  elif n == 3:
    C = [3.29149070e+06, 1.08181730e+04, 3.81893943e+00, 2.61378659e+02]
  elif n == 4:
    C = [3.28016632e+06, 1.08619679e+04, 3.83385365e+00, 2.59776707e+02]
  elif n == 5:
    C = [3.27996990e+06, 1.08658416e+04, 3.83256133e+00, 2.59864404e+02]
  return (C[0]+C[1]*U)/(C[2]*(V-U))-C[3]

# reference temperature: empty, N1, N2, N3, W, L -- THIS MUST BE CLARIFIED!!!
def reference():
  day    = int(ctime().split()[2])
  month = ctime().split()[1]
  rT[1] = float(N1[((N1[:,0] == str(day))*(N1[:,1] == month)).nonzero()[0][0],2])
  rT[2] = float(N2[((N2[:,0] == str(day))*(N2[:,1] == month)).nonzero()[0][0],2])
  rT[3] = float(N3[((N3[:,0] == str(day))*(N3[:,1] == month)).nonzero()[0][0],2])
  rT[4] = float(W[((W[:,0] == str(day))*(W[:,1] == month)).nonzero()[0][0],2])
  rT[5] = float(L[((L[:,0] == str(day))*(L[:,1] == month)).nonzero()[0][0],2])
  
# function decides what power should be supplied to each channel
def power():
  cT = array(T)[-int(PRD*60/prd):,:].mean(0) # current temperature
  WTA.write(ctime()+'\t %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f\n' % tuple(cT))
  WTA.flush()
  print ctime()
  print '   curent T: %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f' % tuple(cT)
  print 'reference T: %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f' % tuple(rT)
  dT1 = rT - cT     # current gradients that are equal to the target heating rate (K per hour)
  dT0 = polyfit(linspace(prd/3600,PRD/60.0,int(PRD*60/prd)), array(T)[-int(PRD*60/prd):,:], 1)[0,:]  # the real temperature evolution K per hour
  # it tryes to reach the target temperature T in one hour
  for p in range(6):
    PW[p] = (3600.0*PW[p] + C[p]*(dT1[p]-dT0[p]))/3600.0  # power in W
    PW[p] = PW[p] if PW[p] <= 120 else 120 # if the requested power exceeds the capacity of the heater (120W)
    PW[p] = PW[p]*(PW[p] > 0) # if the power is negative, that is usual if it cools down too slow
  WP.write(ctime()+'\t %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f\n' % tuple(PW))
  WP.flush()
  print ' heat power: %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f' % tuple(PW)


#########################################
#                                       #
# THIS IS THE MAIN BODY OF THE PROGRAMM #
#                                       #
#########################################

# main settings!!!!
prd = 5.0  # period (sec) of the temperature measuring and the heaters switching ON
PRD = 20   # period (min) of the derivatives calculation and heat power decision making
V = 3298.0 # the reference potential for ADC 
# common variables definition
AQ = aquarium()
T = []                                     # actual temperature
rT = array([2.0, 0.0, 0.0, 0.0, 0.0, 0.0]) # reference temperature
C  = array([4200.0*200, 4200.0*200, 4200.0*200, 4200.0*200, 4200.0*200, 4200.0*200]) # heat cpacity

# output files creation
nameTF = 'TemperatureFull'
nameTA = 'TemperatureAveraged'
nameP  = 'Power'
bkpname = ctime().split()[1]+'-'+ctime().split()[2]+'-bkp'

# full temperature
if os.path.isfile(nameTF+'.log'):
  n = 0
  while os.path.isfile(nameTF+'-'+bkpname+'-%d.log' % n):
    n += 1
  os.system('cp '+nameTF+'.log '+nameTF+'-'+bkpname+'-%d.log' % n)
WTF = open(nameTF+'.log','w')

# averaged temperature
if os.path.isfile(nameTA+'.log'):
  n = 0
  while os.path.isfile(nameTA+'-'+bkpname+'-%d.log' % n):
    n += 1
  os.system('cp '+nameTA+'.log '+nameTA+'-'+bkpname+'-%d.log' % n)
WTA = open(nameTA+'.log','w')

# power
if os.path.isfile(nameP+'.log'):
  # initial power of the heaters
  F = open(nameP+'.log','r')
  lines = F.readlines()
  F.close()
  PW = []
  if len(lines) > 1:
    for p in range(0,12*(len(lines) >= 12 + len(lines)*(len(lines) < 12))):
      PW.append(float32(lines[-p].split()[5:]))
    PW = array(PW).mean(0)
  else:
    PW = array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
  # backupping :)
  n = 0
  while os.path.isfile(nameP+'-'+bkpname+'-%d.log' % n):
    n += 1
  os.system('cp '+nameP+'.log '+nameP+'-'+bkpname+'-%d.log' % n)
else:
  PW = array([0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
WP  = open(nameP+'.log','w')

# reference temperature files
N1 = loadtxt('../reference/N1-smooth.log', dtype='str')
N2 = loadtxt('../reference/N2-smooth.log', dtype='str')
N3 = loadtxt('../reference/N3-smooth.log', dtype='str')
W  = loadtxt('../reference/W-smooth.log', dtype='str')
L  = loadtxt('../reference/L-smooth.log', dtype='str')

# initial reference temperature at sturtup
reference()

# MAIN WHILE
wake = time()
c = 0
while True:
  temp = []
  for p in range(6):
    v = (2*V/(2**5))*(AQ.adc(p) - int('800000', 16))/int('ffffff', 16)
    temp.append(tenz2temp(p,v))
    sleep(0.2)
  T.append(temp) 
  WTF.write(ctime()+'\t')
  WTF.write('%6.3f %6.3f %6.3f %6.3f %6.3f %6.3f\n' % tuple(temp))
  WTF.flush()
  for p in range(6):
    if int(5000.0*PW[p]/120.0) > 0:
      AQ.gpio(5-p, 1, 5000.0*PW[p]/120.0)
      sleep(0.1)
  # this will chop the head of the temperature list
  if len(T) == 10000:
      T = T[5000:]
  # power definition each 10 minutes
  if (c%int(PRD*60/prd) == 0) and (c != 0):
    power()
  # reference update
  tm = int16(ctime().split()[3].split(':'))
  if (tm[0] == 0) and (tm[1] == 0) and (0 < tm[2] < 10):
    reference()
  c += 1
  wake += prd
  sleep(wake-time())

