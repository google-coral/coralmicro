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

#include "PDM.h"

namespace coralmicro {
namespace arduino {

PDMClass::PDMClass()
    : driver_(g_audio_buffers),
      config_{CheckSampleRate(kAudioSampleFrequency).value(), kNumDmaBuffers,
              kDmaBufferSizeMs},
      audio_service_(&driver_, config_, 4, kDropFirstSamplesMs),
      latest_samples_(
          MsToSamples(CheckSampleRate(kAudioSampleFrequency).value(), 1000)) {}

PDMClass::~PDMClass() { end(); }

int PDMClass::begin() {
  current_audio_cb_id_ = audio_service_.AddCallback(
      this, +[](void* ctx, const int32_t* samples, size_t num_samples) -> bool {
        auto pdm = static_cast<PDMClass*>(ctx);
        pdm->Append(samples, num_samples);
        return true;
      });
  return 0;
}

void PDMClass::end() {
  if (current_audio_cb_id_.has_value()) {
    audio_service_.RemoveCallback(current_audio_cb_id_.value());
  }
}

int PDMClass::available() { return latest_samples_.NumSamples(); }

int PDMClass::read(std::vector<int32_t>& buffer, size_t size) {
  buffer = latest_samples_.CopyLatestSamples();
  if (buffer.size() > size) {
    buffer.erase(buffer.begin(), buffer.begin() + (buffer.size() - size));
  }
  return buffer.size();
}

void PDMClass::onReceive(void (*function)(void)) {
  if (function == nullptr) {
    return;
  }
  onReceive_ = function;

  if (current_audio_cb_id_.has_value()) {
    audio_service_.RemoveCallback(current_audio_cb_id_.value());
  }

  current_audio_cb_id_ = audio_service_.AddCallback(
      this, +[](void* ctx, const int32_t* samples, size_t num_samples) -> bool {
        auto pdm = static_cast<PDMClass*>(ctx);
        pdm->Append(samples, num_samples);
        if (pdm->onReceive_) {
          pdm->onReceive_();
        }
        return true;
      });
}

void PDMClass::setGain(int gain) {
  // Not Implemented
}

void PDMClass::setBufferSize(int bufferSize) {
  // Not Implemented
}

void PDMClass::Append(const int32_t* samples, size_t num_samples) {
  latest_samples_.Append(samples, num_samples);
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::PDMClass Mic = coralmicro::arduino::PDMClass();
