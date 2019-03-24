#! /usr/bin/python
import RPi.GPIO as GPIO
import Adafruit_GPIO.I2C as I2C
import threading
import atexit
import time
import struct

class RotaryEncoder():

  registered = False
  encoders = {}

  def __init__(self, address, invert=False, factor=1):
    if RotaryEncoder.registered is False:
      RotaryEncoder.registered = True
      atexit.register(GPIO.cleanup)
      GPIO.setmode(GPIO.BCM)
      GPIO.setup(17, GPIO.IN)
      GPIO.add_event_detect(17, GPIO.FALLING, callback=RotaryEncoder._interrupt)

    self.address = address
    self.factor = factor
    if invert:
      self.factor *= -1

    if address in RotaryEncoder.encoders:
      raise Exception("Encoder at that address already registered")

    self.device = I2C.get_i2c_device(address, busnum=1)
    self.residual = 0
    RotaryEncoder.encoders[address] = self

  def __del__(self):
    del RotaryEncoder.encoders[self.address]

  @classmethod
  def _interrupt(cls, pin):
    for encoder in cls.encoders.keys():
      cls.encoders[encoder]._call_callbacks(pin)

  def _call_callbacks(self, pin):
    try:
      pips = self.device.readS8(0)
    except IOError as e:
      pips = 0
      self.device.readS8(0)

    try: 
      presses = self.device.readU8(1)
    except IOError as e:
      presses = 0
      self.device.readU8(1)

    # Filter
    if abs(pips) > 14:
      pips = 0
    if presses > 3:
      presses = 0

    pips *= self.factor
    pips += self.residual

    if abs(pips) < 1:
      self.residual = pips
      pips = 0
    else:
      self.residual = pips % 1
      pips -= self.residual

    if pips != 0 and self.turn_cb is not None:
      self.turn_cb(self.address, pips)

    if presses != 0 and self.press_cb is not None:
      self.press_cb(self.address, presses)

  def register_callbacks(self, turn=None, press=None):
    self.turn_cb = turn
    self.press_cb = press

  def clear_switches(self, pin=0):
    try:
      self.device.readS8(0)
    except IOError as e:
      self.device.readS8(0)

    try: 
      self.device.readU8(1)
    except IOError as e:
      self.device.readU8(1)

def _press_cb(address, presses):
    print "Button %d pressed %d times" % (address, presses)

def _turn_cb(address, pips):
    print "Knob %d turned %d pips" % (address, pips)

if __name__ == '__main__':
  re = {}
  re[0x10] = RotaryEncoder(0x10, invert=True, factor=0.5)
  re[0x10].clear_switches()
  re[0x10].register_callbacks(turn=_turn_cb, press=_press_cb)
  re[0x11] = RotaryEncoder(0x11, invert=True, factor=0.5)
  re[0x11].clear_switches()
  re[0x11].register_callbacks(turn=_turn_cb, press=_press_cb)
  re[0x12] = RotaryEncoder(0x12, invert=False, factor=0.5)
  re[0x12].clear_switches()
  re[0x12].register_callbacks(turn=_turn_cb, press=_press_cb)
  re[0x13] = RotaryEncoder(0x13, invert=False, factor=0.5)
  re[0x13].clear_switches()
  re[0x13].register_callbacks(turn=_turn_cb, press=_press_cb)
  while True:
    time.sleep(1)
