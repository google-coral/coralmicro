#include "Arduino.h"
#include "PDM.h"

volatile int samplesRead = 0;
std::vector<int32_t> currentSamples;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino PDM!");

  Mic.onReceive(onPDMData);
  Mic.begin();
}

void loop() {
  if (samplesRead) {
    samplesRead = 0;
    Serial.println(currentSamples[0]);
  }
}

void onPDMData() {
  samplesRead = Mic.available();

  if (samplesRead) {
    currentSamples.clear();
    Mic.read(currentSamples, samplesRead);
  }
}