#include "libs/tasks/AudioTask/audio_reader.h"

namespace valiant {

AudioReader::AudioReader(audio::SampleRate sample_rate, int dma_buffer_size_ms,
                         int num_dma_buffers)
    : dma_buffer_size_ms_(dma_buffer_size_ms) {
    const int samples_per_ms = static_cast<int>(sample_rate) / 1000;
    const int samples_per_dma_buffer = dma_buffer_size_ms * samples_per_ms;

    buffer_.resize(samples_per_dma_buffer);
    dma_buffer_.resize(num_dma_buffers * samples_per_dma_buffer);

    ring_buffer_.Create(
        /*xBufferSize=*/samples_per_dma_buffer * num_dma_buffers,
        /*xTriggerLevel=*/samples_per_dma_buffer);
    assert(ring_buffer_.Ok());

    std::vector<int32_t*> dma_buffers;
    for (int i = 0; i < num_dma_buffers; ++i)
        dma_buffers.push_back(dma_buffer_.data() + i * samples_per_dma_buffer);

    AudioTask::GetSingleton()->Enable(sample_rate, dma_buffers.data(), dma_buffers.size(),
                                      samples_per_dma_buffer, this, Callback);
}

AudioReader::~AudioReader() {
    AudioTask::GetSingleton()->Disable();
}

size_t AudioReader::FillBuffer() {
    auto received_size = ring_buffer_.Receive(
        buffer_.data(), buffer_.size(), pdMS_TO_TICKS(2 * dma_buffer_size_ms_));
    if (received_size != buffer_.size()) ++underflow_count_;
    return received_size;
}

void AudioReader::Callback(void* param, const int32_t* buf, size_t size) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    auto* self = reinterpret_cast<AudioReader*>(param);
    auto sent_size =
        self->ring_buffer_.SendFromISR(buf, size, &xHigherPriorityTaskWoken);
    if (size != sent_size) ++self->overflow_count_;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

LatestAudioReader::LatestAudioReader(valiant::audio::SampleRate sample_rate,
                                     int dma_buffer_size_ms,
                                     int num_dma_buffers,
                                     int latest_buffer_size_ms)
    : sample_rate_(sample_rate),
      dma_buffer_size_ms_(dma_buffer_size_ms),
      num_dma_buffers_(num_dma_buffers) {
    auto ret = xTaskCreate(StaticRun, "latest_audio_reader",
                           configMINIMAL_STACK_SIZE * 30, this,
                           APP_TASK_PRIORITY, &task_);
    assert(ret == pdPASS);

    mutex_ = xSemaphoreCreateMutex();
    assert(mutex_);

    samples_.resize(latest_buffer_size_ms *
                    static_cast<uint32_t>(sample_rate_) / 1000);
}

LatestAudioReader::~LatestAudioReader() {
    {
        MutexLock lock(mutex_);
        done_ = true;
    }

    while (eTaskGetState(task_) != eSuspended) taskYIELD();
    vTaskDelete(task_);
    vSemaphoreDelete(mutex_);
}

void LatestAudioReader::StaticRun(void* param) {
    reinterpret_cast<LatestAudioReader*>(param)->Run();
    vTaskSuspend(nullptr);
}

void LatestAudioReader::Run() {
    valiant::AudioReader reader(sample_rate_, dma_buffer_size_ms_,
                                num_dma_buffers_);
    auto& buffer = reader.Buffer();

    bool done = false;
    while (!done) {
        auto size = reader.FillBuffer();
        {
            MutexLock lock(mutex_);
            for (size_t i = 0; i < size; ++i)
                samples_[(pos_ + i) % samples_.size()] = buffer[i];
            pos_ = (pos_ + size) % samples_.size();
            done = done_;
        }
    }
}

}  // namespace valiant
