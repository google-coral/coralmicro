#include "Arduino.h"
#include "Wire.h"

void setup() {
    Wire.begin();
    Serial.begin(115200);
}

void loop() {
    Wire.requestFrom(0x42, 6);

    while (Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
    }

    delay(500);
}
