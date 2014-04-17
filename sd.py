#!/usr/bin/env python

import serial
from datetime import datetime
                                     

print ("Starting Program")
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
print ("ser")

while 1 :
     if(ser.isOpen() == True):
                   # print("ser is open")
                    x = ser.readline()
                    if x :
                        print ("ACM0 is open")
                        print (x.strip())
                        output_file = open("/var/www/curdata1.txt", "ab")
                        output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
                        output_file.write(x)
                        #output_file.write('\n')
                        output_file.close()
