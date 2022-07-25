
#include "Arduino.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("Serial echoing");
}

void loop() {
  if (!Serial.available()) {
    return;
  }
  int ch = Serial.read();
  if (ch != EOF) {
    printf("%c", ch);
    Serial.write(ch);
  }
}
