#include <Arduino.h>

// Runs simple tone cycles with the exposed pwm pins.
// Pin 10 on the left-side header
constexpr int Pin10 = A3;
// pin 9 on the left-side header
constexpr int Pin9 = A4;
// Note: These pins output a max of 1.8V

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino Tone!");
}

void loop() {
  tone(/*pin=*/Pin10, /*frequency=*/1000);
  delay(1000);
  noTone(Pin10);
  tone(/*pin=*/Pin9, /*frequency=*/2000);
  delay(1000);
  noTone(Pin9);
}