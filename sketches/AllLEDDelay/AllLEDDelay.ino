#include "Arduino.h"

int ledPin = PIN_LED_USER;
int powerPin = PIN_LED_POWER;
int tpuPin = PIN_LED_TPU;

void setup() {
    pinMode(ledPin, OUTPUT);
    pinMode(powerPin, OUTPUT);
    pinMode(tpuPin, OUTPUT);
}

void loop() {
    digitalWrite(ledPin, HIGH);
    digitalWrite(powerPin, HIGH);
    digitalWrite(tpuPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    digitalWrite(powerPin, LOW);
    digitalWrite(tpuPin, LOW);
    delay(1000);
}
