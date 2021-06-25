#include "Arduino.h"

int ledPin = A3;
int analogPin = A0;
int val = 0;

void setup() {
}

void loop() {
    val = analogRead(analogPin);
    analogWrite(ledPin, val / 4);
}