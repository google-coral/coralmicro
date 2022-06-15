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

#ifndef PDM_H_
#define PDM_H_

#include <memory>
#include <optional>

#include "Arduino.h"
#include "libs/audio/audio_driver.h"
#include "libs/audio/audio_service.h"

namespace {
// from micro_model_settings.h

// Only 16kHz and 48kHz are supported
constexpr int kAudioSampleFrequency = 16000;

constexpr int kSamplesPerMs = kAudioSampleFrequency / 1000;

constexpr int kNumDmaBuffers = 10;
constexpr int kDmaBufferSizeMs = 100;
constexpr int kDmaBufferSize = kDmaBufferSizeMs * kSamplesPerMs;
coralmicro::AudioDriverBuffers<kNumDmaBuffers,
                                 kNumDmaBuffers * kDmaBufferSize>
    g_audio_buffers;

constexpr int kAudioBufferSizeMs = 1000;
constexpr int kAudioBufferSize = kAudioBufferSizeMs * kSamplesPerMs;
int16_t g_audio_buffer[kAudioBufferSize] __attribute__((aligned(16)));

constexpr int kDropFirstSamplesMs = 150;
}  // namespace

namespace coralmicro {
namespace arduino {

// Exposes the Coral Micro device's native PDM microphone.  
// You should not initialize this object yourself; instead include `PDM.h` and then use the global `Mic` instance.
// The microphone will remain powered and process input whenever there is an active audo callback,
// which you can activate with `onReceive()`, and deactivate with `end()`.
// Example code can be found in `sketches/PDM/`
class PDMClass {
   public:
    // @cond Internal only, do not generate docs.
    PDMClass();
    ~PDMClass();
    // @endcond

    // This function is a no-op.
    //
    int begin();

    // Sets the current audio callback function.  The microphone will run while 
    // there is a callback set, so if there is no current callback, this function
    // effectively turns on the microphone as well.  Within the callback, you can 
    // call `available()` and `read()` to access the microphone data.
    //
    // @param function The function to call when audio data is received.  The function
    // should not have any arguments and not have a return value.
    void onReceive(void (*function)(void));

    // Removes the current callback, effectively turning off the microphone.
    //
    void end();

    // Gets the amount of available data in the audio buffer.
    // Data is stored in the buffer as `uint32_t`s, and the sizes
    // in this function refer to the amount of values in the buffer.
    // 
    // @returns The amount of data values stored in the underlying buffer
    // that are ready to be read.  
    int available();

    // Reads data from the audio buffer.  
    // Data is stored in the buffer as `uint32_t`s, and the sizes
    // in this function refer to the amount of values in the buffer.
    //
    // @param buffer The buffer that will receive the copied audio data.
    // @param size The amount of audio data values to copy.
    // @returns The amount of audio data values that were copied.
    int read(std::vector<int32_t>& buffer, size_t size);

    // @cond Internal only, do not generate docs.
    // Not Implemented
    void setGain(int gain);
    void setBufferSize(int bufferSize);
    // @endcond

   private:
    void Append(const int32_t* samples, size_t num_samples);

    AudioDriver driver_;
    AudioDriverConfig config_;
    AudioService audio_service_;
    LatestSamples latest_samples_;
    std::optional<int> current_audio_cb_id_;

    void (*onReceive_)(void);
};

}  // namespace arduino
}  // namespace coralmicro

// This is the global `PDMClass` instance you should use instead of 
// creating your own instance of `PDMClass`.
extern coralmicro::arduino::PDMClass Mic;

#endif  // PDM_H_
