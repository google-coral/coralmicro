#include "Arduino.h"

int ledPin = PIN_LED_USER;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino Random Delay!");

  pinMode(ledPin, OUTPUT);
}

void loop() {
  for (int s = 0; s < 5; s++) {
    randomSeed(s);
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPin, HIGH);
      long value = random(250, 750);
      Serial.print("random seed: ");
      Serial.print(s);
      Serial.print(" value: ");
      Serial.println(value);

      delay(value);
      digitalWrite(ledPin, LOW);
      delay(1000);
    }
  }
}
