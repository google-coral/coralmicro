#include "Arduino.h"
#include "Wire.h"

void setup() {
  Wire.begin(0x42);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
}

void loop() { delay(100); }

void receiveEvent(int n) {
  while (Wire.available() > 1) {
    char c = Wire.read();
    Serial.print(c);
  }
  int x = Wire.read();
  Serial.println(x);
}
