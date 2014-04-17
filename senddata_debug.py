#!/usr/bin/env python

import serial
from datetime import datetime

def scriptdebug(errormessage):
    with open("error.log", "a") as errorlog:
        errorlog.write("%s\t%s\n" % (datetime.strftime(datetime.now(),
                                     "%d-%m-%y %H:%M"),
                                     errormessage))
									 

try:
	ser = serial.Serial('/dev/ttyACM0', 9600)
	ser1 = serial.Serial('/dev/ttyACM1', 9600)
	while 1 :
		x = ser.readline()
		y = ser1.readline()
		if x :
				print x.strip()
				output_file = open("/var/www/curdata1.txt", "ab")
				output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
				output_file.write(x)
				#output_file.write('\n')
				output_file.close()
		if y :
				print y.strip()
				output_file = open("/var/www/curdata1.txt", "ab")
				output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
				output_file.write(y)
				#output_file.write('\n')
				output_file.close()		
					#code
except Exception, e:
	scriptdebug(e)
