/*
  Blinks the user LED (green) and status LED (orange).
*/

#include "Arduino.h"

void setup() {
  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
}

void loop() {
  digitalWrite(PIN_LED_USER, HIGH);
  digitalWrite(PIN_LED_STATUS, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_USER, LOW);
  digitalWrite(PIN_LED_STATUS, LOW);
  delay(1000);
}
