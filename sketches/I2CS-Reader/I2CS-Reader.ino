#include "Arduino.h"
#include "Wire.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino !");

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
