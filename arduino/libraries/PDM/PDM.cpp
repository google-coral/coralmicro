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
    : config_{nullptr},
      audio_service_{nullptr},
      current_audio_cb_id_{std::nullopt},
      on_receive_{nullptr},
      mutex_(xSemaphoreCreateMutex()),
      read_pos_{0},
      available_{0} {}

PDMClass::~PDMClass() { end(); }

int PDMClass::begin(int sample_rate_hz, size_t sample_size_ms,
                    size_t drop_first_samples_ms) {
  auto sample_rate = CheckSampleRate(sample_rate_hz);
  if (!sample_rate.has_value()) {
    printf("Sample rate: %d not supported\r\n", sample_rate_hz);
    return 0;
  }
  config_ = std::make_unique<AudioDriverConfig>(*sample_rate, kNumDmaBuffers,
                                                kDmaBufferSizeMs);
  if (!audio_buffers_.CanHandle(*config_)) {
    printf("Not enough static memory for DMA buffers\r\n");
    return 0;
  }
  audio_service_ = std::make_unique<AudioService>(
      &driver_, *config_, kAudioServiceTaskPriority, drop_first_samples_ms);
  samples_.clear();
  samples_.resize(MsToSamples(config_->sample_rate, sample_size_ms));
  read_pos_ = 0;
  available_ = 0;
  if (current_audio_cb_id_.has_value()) {
    audio_service_->RemoveCallback(current_audio_cb_id_.value());
  }
  current_audio_cb_id_ = audio_service_->AddCallback(
      this, +[](void* ctx, const int32_t* samples, size_t num_samples) -> bool {
        auto pdm = static_cast<PDMClass*>(ctx);
        pdm->Append(samples, num_samples);
        if (pdm->on_receive_ != nullptr) pdm->on_receive_();
        return true;
      });
  return 1;
}

void PDMClass::end() {
  audio_service_.reset();
  config_.reset();
  if (current_audio_cb_id_.has_value()) {
    audio_service_->RemoveCallback(current_audio_cb_id_.value());
  }
}

int PDMClass::available() { return available_; }

int PDMClass::read(std::vector<int32_t>& buffer, size_t size) {
  // Disallow reading more than what's available.
  auto read = size < available_ ? size : available_;
  buffer.clear();
  {
    MutexLock lock(mutex_);
    std::rotate(std::begin(samples_), std::begin(samples_) + read_pos_,
                std::end(samples_));
    read_pos_ = 0;
    buffer.assign(samples_.begin(), samples_.begin() + read);
    available_ -= read;
    std::rotate(std::begin(samples_), std::begin(samples_) + available_,
                std::end(samples_));
  }
  return read;
}

void PDMClass::onReceive(void (*function)(void)) { on_receive_ = function; }

void PDMClass::setGain(int gain) {
  // Not Implemented
}

void PDMClass::setBufferSize(int bufferSize) {
  // Not Implemented
}

void PDMClass::Append(const int32_t* samples, size_t num_samples) {
  {
    MutexLock lock(mutex_);
    for (size_t i = 0; i < num_samples; ++i)
      samples_[(read_pos_ + i) % samples_.size()] = samples[i];
    read_pos_ = (read_pos_ + num_samples) % samples_.size();
    available_ += num_samples;
    available_ = available_ < samples_.size() ? available_ : samples_.size();
  }
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::PDMClass Mic = coralmicro::arduino::PDMClass();
