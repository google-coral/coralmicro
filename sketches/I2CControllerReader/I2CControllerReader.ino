#include "Arduino.h"
#include "Wire.h"

void setup() {
  Wire.begin();
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino I2C Controller Reader!");
}

void loop() {
  Wire.requestFrom(0x42, 6);

  while (Wire.available()) {
    char c = Wire.read();
    Serial.print(c);
  }

  delay(500);
}
