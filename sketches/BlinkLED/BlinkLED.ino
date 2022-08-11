/*
  Blinks the user LED (green).
*/

#include "Arduino.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino Blink LED!");

  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);

  // Turn on Status LED to show the board is on.
  digitalWrite(PIN_LED_STATUS, HIGH);
}

void loop() {
  digitalWrite(PIN_LED_USER, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_USER, LOW);
  delay(1000);
}
