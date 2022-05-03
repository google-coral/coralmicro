#include "PDM.h"

namespace coral::micro {
namespace arduino {

PDMClass::PDMClass()
    : driver_(g_audio_buffers),
      config_{CheckSampleRate(kAudioSampleFrequency).value(), kNumDmaBuffers,
              kDmaBufferSizeMs},
      audio_service_(&driver_, config_, 4, kDropFirstSamplesMs),
      latest_samples_(
          MsToSamples(CheckSampleRate(kAudioSampleFrequency).value(), 1000)) {}

PDMClass::~PDMClass() { end(); }

int PDMClass::begin() { return 0; }

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
    onReceive_ = function;

    if (current_audio_cb_id_.has_value()) {
        audio_service_.RemoveCallback(current_audio_cb_id_.value());
    }

    audio_service_.AddCallback(
        this,
        +[](void* ctx, const int32_t* samples, size_t num_samples) -> bool {
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
}  // namespace coral::micro

coral::micro::arduino::PDMClass Mic = coral::micro::arduino::PDMClass();