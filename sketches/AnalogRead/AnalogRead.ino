#include "Arduino.h"

int analogPin = A0;
int val = 0;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
}

void loop() {
    val = analogRead(analogPin);
    Serial.println(val);
}
