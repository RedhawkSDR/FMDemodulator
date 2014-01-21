#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#


import numpy as np
import scipy.signal as sp
from ossie.utils import sb
import time
import scipy
import matplotlib.pyplot as plt

print '*****************************************'
print '******* Liquid FM Demod Unit Test *******'
print '*****************************************'


print '\n*********** Generating Data *************'
fs = 256000.0 		# Sample Rate
fm = 250.0 			# Signal Frequency (Hz)
fc = 1000.0 		# Carrier Frequency (Hz)
b = (fc - fm) / fm	# Modulation Index

# Generate a time signal
t = np.arange(0, 1, 1.0 / fs)

# Generate the message signals for modulation
x = np.sin(2 * np.pi * fm * t)

#Modulate the signal
modfm = np.cos(2 * np.pi * fc * t + b * x)

#Apply the Hilbert Transform
complexModFM = sp.hilbert(modfm)

#Data to be pushed
pushComplexModFM = np.ndarray(2 * len(complexModFM))
for i in range(0,len(pushComplexModFM),2):
	pushComplexModFM[i] = complexModFM[i/2].real
	pushComplexModFM[i+1] = complexModFM[i/2].imag

print '\n********* Creating Components ***********'
#Set up components and helpers for the waveform
fmDemod = sb.launch('../FMDemodulator.spd.xml', execparams={'DEBUG_LEVEL':5})
dataSource = sb.DataSource(bytesPerPush=pushComplexModFM.nbytes)
dataSink = sb.DataSink()

print '\n********* Creating Connections **********'
#Make connections
dataSource.connect(fmDemod)
fmDemod.connect(dataSink)
print 'Connections Created'

#Start sandbox env
print '\n*********** Starting Sandbox ************'
sb.start()
print 'Components Started'

#Run unit test
print 'Pushing data'
dataSource.push(pushComplexModFM.tolist(), sampleRate=fs, complexData=False)
time.sleep(1)
output = dataSink.getData()

#Stop sandbox env
print '\n*********** Stopping Sandbox ************'
sb.stop()
print 'Components stopped'

#Index begins when data is no longer skewed
index = 100000
while abs(output[index]) > 0.001:
	index += 1

#Set values up to index to 0 for more accurate results
for i in range(0, index):
	output[i] = 0

# Remove DC Bias from output
bias = sum(output)/float(len(output))
output = [(item-bias) for item in output]

# Scale output to have same magnitude as input
output_max = max(output[index:len(x)])
x_max = max(x[index:len(x)])
gain_adjust = output_max/x_max
output = [(item/gain_adjust) for item in output]

# Shift output to have same phase as input
count = 0
output_i = 0
if output[index] > 0:
	last_sign = 1
else:
	last_sign = -1

for i,j in enumerate(output[index:]):
	if j > 0:
		sign = 1
	else:
		sign = -1

	if last_sign != sign:
		count += 1
		last_sign = sign

	if count == 10:
		output_i = i

count = 0
x_i = 0
if x[index] > 0:
	last_sign = 1
else:
	last_sign = -1

for i, j in enumerate(x[index:]):
	if j > 0:
		sign = 1
	else:
		sign = -1

	if last_sign != sign:
		count += 1
		last_sign = sign
	if count == 10:
		x_i = i

difference = x_i - output_i
output = np.roll(output, difference)

# Plot the data
#plt.plot(x)
#plt.plot(modfm)
#plt.plot(output)
#plt.show()

#Validate results
assert len(x) == len(output), 'Input signal and output result are not equal'
sumError = sum([abs(y-z) for y,z in zip(output[index:],x[index:])])
meanError = sumError/len(output)
#print 'meanError =',meanError
passed = True
if meanError > .1:
	passed = False
                
print '\n************** Results ****************'
if passed:
    print "Unit Test 1 .........................",u'\u2714'
else:
    print "Unit Test 1 .........................",u'\u2718'
    
print '\nTest Complete\n'