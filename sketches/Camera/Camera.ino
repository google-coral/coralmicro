// [start-snippet:ardu-camera]
#include <coralmicro_camera.h>
#include <coralmicro_SD.h>
#include <libs/libjpeg/jpeg.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

FrameBuffer frame_buffer;

void setup() {
  Serial.begin(115200);
  SD.begin();
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera!");
  int width = 324;
  int height = 324;

  if (Camera.begin(width, height) !=
      CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }

  if (!Camera.grab(frame_buffer) == CameraStatus::SUCCESS) {
    return;
  }

  Serial.println("Saving image as \"image.jpeg\"");
  std::vector<uint8_t> jpeg;
  coralmicro::JpegCompressRgb(frame_buffer.getBuffer(), width, height, /*quality=*/75, &jpeg);

  SDFile imageFile = SD.open("image.jpeg", FILE_WRITE);
  imageFile.write(jpeg.data(), jpeg.size());
  imageFile.close();
}

void loop() {}
// [end-snippet:ardu-camera]
