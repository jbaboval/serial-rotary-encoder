#include <TinyWireS.h>

const byte encA   = 10;
const byte encB   = 9;
const byte encSw  = 8;
const byte ledB   = 2;
const byte ledG   = 1;
const byte ledR   = 0;
const byte jp2    = 3;
const byte intr   = 7;

volatile byte pips = 0;
volatile byte presses = 0;
volatile byte A = 1;
volatile byte B = 1;
volatile byte Sw = 1;
volatile byte reg = 0;
volatile byte index = 0;
volatile byte debounce = 0;

#define digitalPinToInterrupt(p) ( (p <= 7) ? (p) : (10 - (p - 8)) )

void setup() {
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(encA, INPUT);
  pinMode(encB, INPUT);
  pinMode(encSw,INPUT);
  pinMode(intr, OUTPUT);

  digitalWrite(encA, HIGH);
  digitalWrite(encB, HIGH);
  digitalWrite(encSw,HIGH);
  digitalWrite(intr, LOW);

  TinyWireS.begin(0x10);
  TinyWireS.onReceive(onWrite);
  TinyWireS.onRequest(onRead);

  GIMSK |= 0x20;
  PCMSK1 |= 0x07;

//  attachInterrupt(digitalPinToInterrupt(encSw), buttonPressed, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(encA),  knobTurnedA,   CHANGE);
//  attachInterrupt(digitalPinToInterrupt(encB),  knobTurnedB,   CHANGE);
}

void onWrite(byte count) {
  byte i;
  reg = TinyWireS.receive();
  count--;
  digitalWrite(ledR, reg & 0x10);
  digitalWrite(ledG, reg & 0x20);
  digitalWrite(ledB, reg & 0x40);
  while (count) {
    TinyWireS.receive();
    count--;
  }
}

void onRead() {
  if (reg & 0x1) {
    index = 1;
  } else if (reg & 0x2) {
    index = 2;
  }
  switch (index) {
    case 3:
      index = 0;
    case 0:
      TinyWireS.send('E');
      break;
    case 1:
      TinyWireS.send(pips);
      pips = 0;
      digitalWrite(intr, LOW);
      break;
    case 2:
      TinyWireS.send(presses);
      presses = 0;
      digitalWrite(intr, LOW);
      break;
  }
  index++;
}

ISR(PCINT1_vect) {
  byte intA  = digitalRead(encA);
  byte intB  = digitalRead(encB);
  byte intSw = digitalRead(encSw);
  byte which = 0;

  if (intSw != Sw && !intSw && !debounce) {
    presses++;
    digitalWrite(intr, HIGH);
    debounce = 50;
  }

  Sw = intSw;

  if (intA == A && intB == B) {
    return;
  } else if ( intB != B) {
    which = 1;
  }

  A = intA;
  B = intB;

  if ( A && B ) {
    if (which) {
      pips++;
    } else {
      pips--;
    }
    digitalWrite(intr, HIGH);
  }
}

void loop() {
  tws_delay(5);
  if (debounce) {
    debounce--;
  }
}
