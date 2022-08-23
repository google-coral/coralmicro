#include "Arduino.h"
#include "Wire.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino I2C Target Writer!");

  Wire.begin(0x42);
  Wire.onRequest(requestEvent);
}

void loop() { delay(100); }

void requestEvent() { Wire.write("hello "); }
