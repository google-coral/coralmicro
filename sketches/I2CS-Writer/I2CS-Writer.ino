#include "Arduino.h"
#include "Wire.h"

void setup() {
    Wire.begin(0x42);
    Wire.onRequest(requestEvent);
}

void loop() {
    delay(100);
}

void requestEvent() {
    Wire.write("hello ");
}
