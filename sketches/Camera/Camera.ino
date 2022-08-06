// [start-snippet:ardu-camera]
#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

FrameBuffer frame_buffer;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Camera!");

  if (Camera.begin(CameraResolution::CAMERA_R324x324) !=
      CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }
}

void loop() {
  Camera.setTestPattern(true, coralmicro::CameraTestPattern::kWalkingOnes);
  if (Camera.grab(frame_buffer) == CameraStatus::SUCCESS) {
    bool success{true};
    uint8_t expected{0};
    auto* frame_data = frame_buffer.getBuffer();
    for (uint32_t idx = 0; idx < frame_buffer.getBufferSize(); ++idx) {
      char buf[100];
      if (frame_data[idx] != expected) {
        sprintf(buf,
                "[ERROR] Test pattern mismatch at index %lu... 0x%x != "
                "0x%x\r\n",
                idx, frame_data[idx], expected);
        Serial.println(buf);
        success = false;
        break;
      }
      if (expected == 0) {
        expected = 1;
      } else {
        expected = expected << 1;
      }
    }
    if (success) {
      Serial.println("Test pattern PASSED");
    } else {
      Serial.println("Test pattern FAILED");
    }
  } else {
    Serial.println("Failed to get image data");
  }
  delay(5000);
}
// [end-snippet:ardu-camera]
