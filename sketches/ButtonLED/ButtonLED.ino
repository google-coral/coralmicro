/*
  Turns the user LED (green) on when you press the user button
  on the Coral Dev Board Micro.
*/

#include "Arduino.h"

int ledPin = PIN_LED_USER;
int buttonPin = PIN_BTN;
PinStatus val = LOW;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino Button LED!");

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  val = digitalRead(buttonPin);
  digitalWrite(ledPin, val == LOW ? HIGH : LOW);
}
