#include "Arduino.h"
#include "SPI.h"

static uint8_t count8 = 0x1;
static uint16_t count16 = 0x101;
static const size_t transferSize = 256U;
uint8_t masterRxData[transferSize];
uint8_t masterTxData[transferSize];

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino SPI!");

  SPI.begin();

  // Initialize send buffer
  for (int i = 0; i < transferSize; i++) {
    masterTxData[i] = i;
    masterRxData[i] = i;
  }
}

void loop() {
  // Testing that the output is the same as the input by connecting SDO to SDI

  Serial.print("1 byte transfer: ");
  Serial.println(SPI.transfer(count8));
  count8++;
  count8 %= 255;

  Serial.print("2 byte transfer: ");
  Serial.println(SPI.transfer16(count16));
  count16 += 0x100;
  if (count16 == 0xFFFF) {
    count16 = 0;
  }

  SPI.transfer(&masterTxData, transferSize);

  bool bufferMatch = true;
  for (int i = 0; i < transferSize; i++) {
    if (masterTxData[i] != masterRxData[i]) {
      Serial.println("256 byte transfer failed");
      bufferMatch = false;
      break;
    }
  }
  if (bufferMatch) {
    Serial.println("256 byte transfer matches");
  }

  delay(1000);
}