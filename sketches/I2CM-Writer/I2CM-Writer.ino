#include "Arduino.h"
#include "Wire.h"

void setup() {
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
