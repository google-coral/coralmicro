#include "Arduino.h"

int ledPin = PIN_LED_USER;
int inPin = PIN_BTN;
PinStatus val = LOW;

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(inPin, INPUT);
}

void loop() {
    val = digitalRead(inPin);
    digitalWrite(ledPin, val);
}
