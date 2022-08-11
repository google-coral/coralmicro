#include "Arduino.h"
#include "SPI.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino SPI Transaction!");

  SPI.begin();
}

void loop() {
  // Testing that the output is the same as the input by connecting SDO to SDI
  delay(1000);

  uint32_t clockFreq = CLOCK_GetFreqFromObs(CCM_OBS_LPSPI6_CLK_ROOT);

  for (int bitOrder = 0; bitOrder <= 1; bitOrder++) {
    for (int spiMode = 0; spiMode <= 3; spiMode++) {
      SPI.updateSettings(arduino::SPISettings(clockFreq, BitOrder(bitOrder),
                                              arduino::SPIMode(spiMode)));
      Serial.println(SPI.transfer16(0x1234));

      Serial.println(SPI.transfer(0x01));
    }
  }
}