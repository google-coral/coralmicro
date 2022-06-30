#include "Arduino.h"
#include "coral_micro.h"

int ledPin = PIN_LED_USER;
int statusPin = PIN_LED_STATUS;
int tpuPin = PIN_LED_TPU;

std::shared_ptr<coral::micro::EdgeTpuContext> context = nullptr;
void setup() {
    // Enable TPU for LED power
    context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
    pinMode(ledPin, OUTPUT);
    pinMode(statusPin, OUTPUT);
}

void loop() {
    digitalWrite(ledPin, HIGH);
    digitalWrite(statusPin, HIGH);
    analogWrite(tpuPin, 255);
    delay(1000);
    digitalWrite(ledPin, LOW);
    digitalWrite(statusPin, LOW);
    analogWrite(tpuPin, 0);
    delay(1000);
}
