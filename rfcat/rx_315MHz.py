#!/usr/bin/python

from rflib import *

#-------------------------------------------------------------------------------
frequancy	= 315000000
modulation	= MOD_ASK_OOK
datarate	= 4800
#-------------------------------------------------------------------------------

d = RfCat()
d.setFreq(frequancy)
d.setMdmModulation(modulation)
d.setMdmDRate(datarate)

d.setMaxPower()
d.lowball()
#-------------------------------------------------------------------------------

print "Starting Reception..."

