#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

bool motion_detected = false;

void on_motion(void*) { motion_detected = true; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_USER, OUTPUT);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  if (Camera.begin(CameraResolution::CAMERA_R324x324) !=
      CameraStatus::SUCCESS) {
    Serial.println("Failed to start camera");
    return;
  }
  Camera.setMotionDetectionWindow(0, 0, 320, 320);
  Camera.enableMotionDetection(on_motion);
  Serial.println("Camera Initialed!");
}

void loop() {
  if (motion_detected) {
    Serial.println("Motion Detected!");
    digitalWrite(PIN_LED_USER, HIGH);
    motion_detected = false;
  } else {
    digitalWrite(PIN_LED_USER, LOW);
  }
  delay(100);
}
