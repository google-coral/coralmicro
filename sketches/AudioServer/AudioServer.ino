/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Arduino.h>
#include <PDM.h>
#include <SocketClient.h>
#include <SocketServer.h>

// This is the arduino sketch equivalent for example/audio_server. Upload
// this sketch and then start an audio client over USB from a linux computer:
//
// Then receive the audio stream over USB from a Linux computer:
//    python3 -m pip install -r examples/audio_server/requirements.txt
//    python3 examples/audio_server/audio_client.py

namespace {

coralmicro::arduino::SocketClient client;
coralmicro::arduino::SocketServer server(33000);

std::vector<int32_t> current_samples;
}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino SocketServer!");
  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  // Turn on Status LED to show the board is on.
  digitalWrite(PIN_LED_STATUS, HIGH);

  server.begin();
  Serial.println("Server ready on port 33000");
  client = server.available();
}

void loop() {
  if (client && client.available()) {
    Serial.println("Client connected");
    int32_t params[2];
    client.read(reinterpret_cast<uint8_t*>(params), 2 * sizeof(int32_t));
    const int sample_rate_hz = params[0];
    const int sample_format = params[1];
    Serial.println("Format:");
    Serial.print("  Sample rate: ");
    Serial.println(sample_rate_hz);
    Serial.print("  Sample format: ");
    Serial.println(sample_format == 0 ? "S16_LE" : "S32_LE");
    Serial.println("Sending audio samples...");
    Serial.flush();

    if (!Mic.begin(sample_rate_hz)) {
      Serial.println("Failed to initialize PDM.");
    }

    uint32_t totalBytes{0};
    while (client.available()) {
      current_samples.clear();
      auto available = Mic.available();
      Mic.read(current_samples, available);
      if (sample_format == 1) {  // S32_LE
        auto bytes = available * sizeof(int32_t);
        if (client.write(reinterpret_cast<uint8_t*>(current_samples.data()),
                         bytes) < 0) {
          Serial.println("Error while sending audio data");
          break;
        }
        totalBytes += bytes;
      } else {  // S16_LE
        std::vector<int16_t> current_samples16(available);
        for (size_t i = 0; i < available; ++i)
          current_samples16[i] = current_samples[i] >> 16;
        auto bytes = available * sizeof(int16_t);
        if (client.write(reinterpret_cast<uint8_t*>(current_samples16.data()),
                         bytes) < 0) {
          Serial.println("Error while sending audio data");
          break;
        }
        totalBytes += bytes;
      }
    }
    Serial.print("Bytes sent: ");
    Serial.println(totalBytes);
    Serial.println("Done.");
  }
}
