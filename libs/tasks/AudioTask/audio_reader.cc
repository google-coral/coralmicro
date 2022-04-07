#include "libs/tasks/AudioTask/audio_reader.h"

#include <memory>

#define CHECK(a)                                                          \
    do {                                                                  \
        if (!(a)) {                                                       \
            printf("%s:%d %s was not true.\r\n", __FILE__, __LINE__, #a); \
            vTaskSuspend(nullptr);                                        \
        }                                                                 \
    } while (0)

namespace valiant {
namespace {
enum class MessageType : uint8_t {
    kAddCallback,
    kRemoveCallback,
    kStop,
};

struct Message {
    MessageType type;
    QueueHandle_t queue;
    union {
        struct {
            void* ctx;
            AudioServiceCallback fn;
        } add;

        struct {
            int id;
        } remove;
    };
};

struct Callback {
    int id;
    void* ctx;
    AudioServiceCallback fn;
};

bool EraseCallbackById(std::vector<Callback>& callbacks, int id) {
    auto it = std::find_if(std::begin(callbacks), std::end(callbacks),
                           [id](const auto& cb) { return cb.id == id; });
    if (it == std::end(callbacks)) return false;
    callbacks.erase(it);
    return true;
}
}  // namespace

AudioReader::AudioReader(audio::SampleRate sample_rate, int dma_buffer_size_ms,
                         int num_dma_buffers)
    : dma_buffer_size_ms_(dma_buffer_size_ms) {
    const int samples_per_dma_buffer =
        audio::MsToSamples(sample_rate, dma_buffer_size_ms);

    buffer_.resize(samples_per_dma_buffer);
    dma_buffer_.resize(num_dma_buffers * samples_per_dma_buffer);

    ring_buffer_.Create(
        /*xBufferSize=*/samples_per_dma_buffer * num_dma_buffers,
        /*xTriggerLevel=*/samples_per_dma_buffer);
    CHECK(ring_buffer_.Ok());

    std::vector<int32_t*> dma_buffers;
    for (int i = 0; i < num_dma_buffers; ++i)
        dma_buffers.push_back(dma_buffer_.data() + i * samples_per_dma_buffer);

    AudioTask::GetSingleton()->Enable(sample_rate, dma_buffers.data(),
                                      dma_buffers.size(),
                                      samples_per_dma_buffer, this, Callback);
}

AudioReader::~AudioReader() { AudioTask::GetSingleton()->Disable(); }

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

AudioService::AudioService(audio::SampleRate sample_rate,
                           int dma_buffer_size_ms, int num_dma_buffers)
    : sample_rate_(sample_rate),
      dma_buffer_size_ms_(dma_buffer_size_ms),
      num_dma_buffers_(num_dma_buffers),
      queue_(xQueueCreate(5, sizeof(Message))) {
    CHECK(queue_);
    CHECK(xTaskCreate(StaticRun, "audio_service", configMINIMAL_STACK_SIZE * 30,
                      this, APP_TASK_PRIORITY, &task_) == pdPASS);
}

AudioService::~AudioService() {
    Message msg{};
    msg.type = MessageType::kStop;
    CHECK(xQueueSendToBack(queue_, &msg, portMAX_DELAY) == pdTRUE);

    while (eTaskGetState(task_) != eSuspended) taskYIELD();
    vTaskDelete(task_);

    vQueueDelete(queue_);
}

int AudioService::AddCallback(void* ctx, AudioServiceCallback fn) {
    Message msg{};
    msg.type = MessageType::kAddCallback;
    msg.queue = xQueueCreate(1, sizeof(int));
    msg.add.ctx = ctx;
    msg.add.fn = fn;
    CHECK(xQueueSendToBack(queue_, &msg, portMAX_DELAY) == pdTRUE);

    int id;
    CHECK(xQueueReceive(msg.queue, &id, portMAX_DELAY) == pdTRUE);
    vQueueDelete(msg.queue);
    return id;
}

bool AudioService::RemoveCallback(int id) {
    Message msg{};
    msg.type = MessageType::kRemoveCallback;
    msg.queue = xQueueCreate(1, sizeof(int));
    msg.remove.id = id;
    CHECK(xQueueSendToBack(queue_, &msg, portMAX_DELAY) == pdTRUE);

    int found;
    CHECK(xQueueReceive(msg.queue, &found, portMAX_DELAY) == pdTRUE);
    vQueueDelete(msg.queue);
    return found;
}

void AudioService::StaticRun(void* param) {
    reinterpret_cast<const AudioService*>(param)->Run();
    vTaskSuspend(nullptr);
}

void AudioService::Run() const {
    std::vector<Callback> callbacks;
    callbacks.reserve(3);

    std::vector<int> callbacks_to_remove;
    callbacks_to_remove.reserve(3);

    std::unique_ptr<AudioReader> reader;

    int id_counter = 0;

    Message msg;
    while (true) {
        const auto timeout_ticks = callbacks.empty() ? portMAX_DELAY : 0;
        if (xQueueReceive(queue_, &msg, timeout_ticks) == pdTRUE) {
            switch (msg.type) {
                case MessageType::kAddCallback: {
                    int id = id_counter++;
                    callbacks.push_back({id, msg.add.ctx, msg.add.fn});
                    CHECK(xQueueSendToBack(msg.queue, &id, portMAX_DELAY) ==
                          pdTRUE);
                } break;

                case MessageType::kRemoveCallback: {
                    int found = EraseCallbackById(callbacks, msg.remove.id);
                    CHECK(xQueueSendToBack(msg.queue, &found, portMAX_DELAY) ==
                          pdTRUE);
                } break;
                case MessageType::kStop:
                    return;
            }
            continue;
        }

        if (callbacks.empty()) {
            if (reader) reader.reset();
            continue;
        }

        if (!reader)
            reader = std::make_unique<AudioReader>(
                sample_rate_, dma_buffer_size_ms_, num_dma_buffers_);

        // Blocks until buffer is full or timeout.
        auto size = reader->FillBuffer();

        callbacks_to_remove.clear();
        for (const auto& cb : callbacks)
            if (!cb.fn(cb.ctx, reader->Buffer().data(), size))
                callbacks_to_remove.push_back(cb.id);

        for (int id : callbacks_to_remove) EraseCallbackById(callbacks, id);

        if (callbacks.empty()) reader.reset();
    }
}

LatestSamples::LatestSamples(size_t num_samples)
    : mutex_(xSemaphoreCreateMutex()), samples_(num_samples) {
    CHECK(mutex_);
}

LatestSamples::~LatestSamples() { vSemaphoreDelete(mutex_); }

}  // namespace valiant
