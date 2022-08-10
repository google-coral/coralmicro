#include "Arduino.h"
#include "Wire.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino i2c Writer!");

  Wire.begin();
  Wire.setClock(400000);
}

uint8_t x = 0;
void loop() {
  Wire.beginTransmission(0x42);
  Wire.write("x is ");
  Wire.write(x);
  Wire.endTransmission();

  x++;
  delay(500);
}
