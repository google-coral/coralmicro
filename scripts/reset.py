#!/usr/bin/python3

import serial

serialPath = '/dev/valiant_UART'
s = serial.Serial(serialPath, baudrate=1200)
s.dtr = False
s.close()
