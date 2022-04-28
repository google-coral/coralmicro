#include "Arduino.h"
#include "coral_micro.h"

int ledPin = PIN_LED_USER;
int powerPin = PIN_LED_POWER;
int tpuPin = PIN_LED_TPU;

std::shared_ptr<coral::micro::EdgeTpuContext> context = nullptr;
void setup() {
    // Enable TPU for LED power
    context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
    pinMode(ledPin, OUTPUT);
    pinMode(powerPin, OUTPUT);
}

void loop() {
    digitalWrite(ledPin, HIGH);
    digitalWrite(powerPin, HIGH);
    analogWrite(tpuPin, 255);
    delay(1000);
    digitalWrite(ledPin, LOW);
    digitalWrite(powerPin, LOW);
    analogWrite(tpuPin, 0);
    delay(1000);
}
