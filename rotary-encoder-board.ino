#include <TinyWireS.h>

const byte encA   = 10;
const byte encB   = 9;
const byte encSw  = 8;
const byte ledB   = 2;
const byte ledG   = 1;
const byte ledR   = 0;
const byte jp2    = 3;
const byte intr   = 7;

#define REG_PIPS    0
#define REG_PRESSES 1
#define REG_LED     2
#define REG_MAX     3

volatile byte reg = 0;
volatile byte registers[REG_MAX];
volatile byte debounce = 0;
volatile byte debounceA = 0;
volatile byte debounceB = 0;

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
}

void onWrite(byte count) {
  byte i;
  reg = TinyWireS.receive();
  count--;
  while (count) {
    registers[reg] = TinyWireS.receive();
    count--;
    if (reg == REG_LED) {
        PORTA &= 0xF4;
        PORTA |= registers[reg] & 0x03;
    }
    reg++;
    reg %= REG_MAX;
  }
}

void onRead() {
      TinyWireS.send(registers[reg]);
      registers[reg] = 0;
      reg++;
      reg %= REG_MAX;
}

ISR(PCINT1_vect) {
  byte intEnc = PINB & 0x03;
  byte intSw = PINB & 0x04;
  static byte Sw = 0;
  static byte enc = 0;
  static byte done = false;
  byte change = intEnc ^ enc;

  if (intSw != Sw && !intSw && !debounce) {
    registers[REG_PRESSES]++;
    debounce = 25;
    digitalWrite(intr, HIGH);
  }

  Sw = intSw;

  if (change & 0x01 && debounceA) {
    return;
  } else if (change & 0x02 && debounceB) {
    return;
  }

  if (change & 0x01) {
    debounceA = 50;
  }

  if (change & 0x02) {
    debounceB = 50;
  }

  if (change && (intEnc == 0x00 || intEnc == 0x03)) {
    digitalWrite(intr, HIGH);
    done = true;
    debounceA = 2;
    debounceB = 2;
  }

  if (done) {
    switch (change) {
      case 0x01:
        registers[REG_PIPS]--;
        break;
      case 0x02:
        registers[REG_PIPS]++;
        break;
    }
  }

  done = false;
  enc = intEnc;
}

void loop() {
  tws_delay(5);
  if (debounce) {
    debounce--;
  }
  if (debounceA) {
    debounceA--;
  }
  if (debounceB) {
    debounceB--;
  }
}
