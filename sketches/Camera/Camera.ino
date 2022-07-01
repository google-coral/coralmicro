#include <coralmicro_camera.h>

#include <cstdint>
#include <memory>

#include "Arduino.h"

using namespace coralmicro::arduino;

constexpr int32_t width = 320;
constexpr int32_t height = 320;
constexpr int32_t channels = 3;
auto frame_buffer = std::make_unique<uint8_t[]>(width * height * channels);

void setup() {
    Serial.begin(115200);
    if (Camera.begin(width, height) != CameraStatus::SUCCESS) {
        Serial.println("Failed to start camera");
        return;
    }
}

void loop() {
    Camera.testPattern(coralmicro::camera::TestPattern::kWalkingOnes);
    Serial.println("Discarding 100 frames...");
    Camera.discardFrames(100);
    delay(5000);

    if (Camera.grab(frame_buffer.get()) == CameraStatus::SUCCESS /*0*/) {
        bool success{true};
        uint8_t expected{0};
        for (uint32_t idx = 0; idx < (width * height); ++idx) {
            char buf[100];
            if (frame_buffer.get()[idx] != expected) {
                sprintf(buf,
                        "[ERROR] Test pattern mismatch at index %lu... 0x%x != "
                        "0x%x\r\n",
                        idx, frame_buffer.get()[idx], expected);
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
