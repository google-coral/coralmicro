
#include "Arduino.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino Serial!");

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
