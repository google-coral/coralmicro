#include <Arduino.h>

// Runs simple tone cycles with the exposed pwm pins.
// Pin 10 on the left-side header
constexpr int Pin10 = A3;
// pin 9 on the left-side header
constexpr int Pin9 = A4;
// Note: These pins output a max of 1.8V

void setup() {}

void loop() {
  tone(/*pin=*/Pin10, /*frequency=*/1000);
  delay(1000);
  noTone(Pin10);
  tone(/*pin=*/Pin9, /*frequency=*/2000);
  delay(1000);
  noTone(Pin9);
}