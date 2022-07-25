#include "Arduino.h"
#include "PDM.h"

volatile int samplesRead = 0;
std::vector<int32_t> currentSamples;

void setup() {
  Serial.begin(115200);

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