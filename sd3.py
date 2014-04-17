#!/usr/bin/env python

import serial
from datetime import datetime
import os.path

                                     

print ("Starting Program")
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=2)
print ("ser")

ser1 = serial.Serial('/dev/ttyACM1', 9600, timeout=2)
print("ser1")

ser2 = serial.Serial('/dev/ttyACM2', 9600, timeout=2)
print("ser2")

while 1 :
     if(ser.isOpen() == True):
                   # print("ser is open")
                    x = ser.readline()
                    y = ser1.readline()
                    z = ser2.readline()
                    if x :
                        print ("ACM0 is open")
                        print (x.strip())
                        if not os.path.isfile("/var/www/curdata1.txt"):
                            output_file = open("/var/www/curdata1.txt", "ab")
                            output_file.write("Date Time,xxx,ID,Outside Temp,Outside Humidity,Tank Temp,pH Sensor Value,pH calculated,IR light #1,Full Light #1,Lux #1,IR Light #2,Full Light #2,Lux #2, Tank Temp #2, pH Sensor Val #2, pH Calculated #2, IR light #3,Full Light #3,Lux #3"+ '\n')
                            output_file.close()
                            
                        output_file = open("/var/www/curdata1.txt", "ab")
                        output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
                        output_file.write(x)
                        output_file.close()
     if(ser1.isOpen() == True):
                   # print("ser2 is open")
             #       y = ser1.readline()
                    if y :
                        print ("ACM1 is open")
                        print (y.strip())
                        if not os.path.isfile("/var/www/curdata1.txt"):
                            output_file = open("/var/www/curdata1.txt", "ab")
                            output_file.write("Date Time,xxx,ID,Outside Temp,Outside Humidity,Tank Temp,pH Sensor Value,pH calculated,IR light #1,Full Light #1,Lux #1,IR Light #2,Full Light #2,Lux #2, Tank Temp #2, pH Sensor Val #2, pH Calculated #2, IR light #3,Full Light #3,Lux #3"+ '\n')
                            output_file.close()
                            
                        output_file = open("/var/www/curdata1.txt", "ab")
                        output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
                        output_file.write(y)
                        output_file.close()
     if(ser2.isOpen() == True):
                   # print("ser3 is open")
                #    z = ser2.readline()
                    if z :
                        print ("ACM2 is open")
                        print (z.strip())
                        if not os.path.isfile("/var/www/curdata1.txt"):
                            output_file = open("/var/www/curdata1.txt", "ab")
                            output_file.write("Date Time,xxx,ID,Outside Temp,Outside Humidity,Tank Temp,pH Sensor Value,pH calculated,IR light #1,Full Light #1,Lux #1,IR Light #2,Full Light #2,Lux #2, Tank Temp #2, pH Sensor Val #2, pH Calculated #2, IR light #3,Full Light #3,Lux #3"+ '\n')
                            output_file.close()
                            
                        output_file = open("/var/www/curdata1.txt", "ab")
                        output_file.write(datetime.strftime(datetime.now(),"%d-%m-%y %H:%M"))
                        output_file.write(z)
                        output_file.close()

                
