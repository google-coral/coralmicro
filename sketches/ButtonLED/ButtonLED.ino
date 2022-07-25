/*
  Turns the user LED (green) on when you press the user button
  on the Coral Dev Board Micro.
*/

#include "Arduino.h"

int ledPin = PIN_LED_USER;
int buttonPin = PIN_BTN;
PinStatus val = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  val = digitalRead(buttonPin);
  digitalWrite(ledPin, val == LOW ? HIGH : LOW);
}
