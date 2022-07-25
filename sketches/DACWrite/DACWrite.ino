#include "Arduino.h"

int dacPin = DAC0;
int analogPin = A0;
int val = 0;

void setup() {}

void loop() {
  val = analogRead(analogPin);
  analogWrite(dacPin, val / 4);
}
