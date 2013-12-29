#!/usr/bin/env python

import pyudev, os, sys, time

import usb.core
import usb.util

# find our device
dev = usb.core.find(idVendor=0x03eb, idProduct=0x2040)

# was it found?
if dev is not None:
    print('Found DevBoard: resetting')
    dev = usb.core.find(idVendor=0x03eb, idProduct=0x2040)
    dev.ctrl_transfer(usb.util.CTRL_TYPE_CLASS | usb.util.CTRL_RECIPIENT_DEVICE,0x01)
    time.sleep(4)

try:
    filename=sys.argv[1]
except IndexError:
    filename="usbdev.hex"

a=pyudev.Context()
c=a.list_devices(subsystem='tty',ID_VENDOR="Arduino_LLC")

try:
    g=c.__iter__().next()
except StopIteration:
    print("not found Arduino bootloader")
    exit(1)

tty=g.get('DEVNAME')

os.execlp("avrdude","avrdude","-c","avr109","-p","m32u4","-P",tty,"-Uflash:w:"+filename)
