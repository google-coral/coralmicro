#include "libs/base/http_server.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "libs/base/strings.h"
#include "libs/base/utils.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

#if defined(OOBE_DEMO_ETHERNET)
#include "libs/base/ethernet.h"
#endif  // defined(OOBE_DEMO_ETHERNET)

#if defined(OOBE_DEMO_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(OOBE_DEMO_WIFI)

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <numeric>
#include <vector>

namespace coral::micro {
namespace {
constexpr float kThreshold = 0.4;

constexpr int kCmdStart = 1;
constexpr int kCmdStop = 2;
constexpr int kCmdProcess = 3;

SemaphoreHandle_t camera_output_mtx;
std::vector<uint8_t> camera_output;

SemaphoreHandle_t posenet_output_mtx;
std::unique_ptr<coral::micro::posenet::Output> posenet_output;
TickType_t posenet_output_time;

std::vector<uint8_t> CreatePoseJson(const posenet::Output& output,
                                    float threshold) {
    std::vector<uint8_t> s;
    s.reserve(2048);

    int num_appended_poses = 0;
    StrAppend(&s, "[");
    for (int i = 0; i < output.num_poses; ++i) {
        if (output.poses[i].score < threshold) continue;

        StrAppend(&s, num_appended_poses != 0 ? ",\n" : "");
        StrAppend(&s, "{\n");
        StrAppend(&s, "  \"score\": %g,\n", output.poses[i].score);
        StrAppend(&s, "  \"keypoints\": [\n");
        for (int j = 0; j < posenet::kKeypoints; ++j) {
            const auto& kp = output.poses[i].keypoints[j];
            StrAppend(&s, "    [%g, %g, %g]", kp.score, kp.x, kp.y);
            StrAppend(&s, j != posenet::kKeypoints - 1 ? ",\n" : "\n");
        }
        StrAppend(&s, "  ]\n");
        StrAppend(&s, "}");
        ++num_appended_poses;
    }
    StrAppend(&s, "]");
    return s;
}

void GenerateStatsHtml(std::vector<uint8_t>* html) {
    std::vector<TaskStatus_t> infos(uxTaskGetNumberOfTasks());
    unsigned long total_runtime;
    auto n = uxTaskGetSystemState(infos.data(), infos.size(), &total_runtime);
    std::vector<int> indices(infos.size());
    std::iota(std::begin(indices), std::end(indices), 0);
    std::sort(std::begin(indices), std::end(indices), [&infos](int i, int j) {
        return infos[i].ulRunTimeCounter > infos[j].ulRunTimeCounter;
    });

    StrAppend(html, "<!DOCTYPE html>\r\n");
    StrAppend(html, "<html lang=\"en\">\r\n");
    StrAppend(html, "<head>\r\n"),
        StrAppend(html, "<title>Run-time statistics</title>\r\n"),
        StrAppend(html, "  <style>\r\n"),
        StrAppend(html, "    th,td {padding: 2px;}\r\n"),
        StrAppend(html, "  </style>\r\n"), StrAppend(html, "</head>\r\n"),
        StrAppend(html, "<body>\r\n");
    StrAppend(html, "  <table>\r\n");
    StrAppend(html,
              "    <tr><th>Task</th><th>Abs Time</th><th>% Time</th></tr>\r\n");
    for (auto i = 0u; i < n; ++i) {
        const auto& info = infos[indices[i]];
        auto name = info.pcTaskName;
        auto runtime = info.ulRunTimeCounter;
        auto percent = runtime / (total_runtime / 100);
        if (percent > 0) {
            StrAppend(html,
                      "    <tr><td>%s</td><td>%d</td><td>%d%%</td></tr>\r\n",
                      name, runtime, percent);
        } else {
            StrAppend(html,
                      "    <tr><td>%s</td><td>%d</td><td>&lt;1%%</td></tr>\r\n",
                      name, runtime);
        }
    }
    StrAppend(html, "  </table>\r\n");
    StrAppend(html, "</body>\r\n");
}

HttpServer::Content UriHandler(const char* name) {
    if (std::strcmp(name, "/stats.html") == 0) {
        std::vector<uint8_t> html;
        html.reserve(2048);
        GenerateStatsHtml(&html);
        return html;
    }

    if (std::strcmp(name, "/camera") == 0) {
        coral::micro::MutexLock lock(camera_output_mtx);
        if (camera_output.empty()) return {};
        return coral::micro::HttpServer::Content{std::move(camera_output)};
    }

    if (std::strcmp(name, "/pose") == 0) {
        coral::micro::MutexLock lock(posenet_output_mtx);
        if (!posenet_output) return {};
        auto json = CreatePoseJson(*posenet_output, kThreshold);
        posenet_output.reset();
        return json;
    }

    return {};
}

void HandleAppMessage(
    const uint8_t data[coral::micro::ipc::kMessageBufferDataSize],
    void* param) {
    (void)data;
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

struct TaskMessage {
    int command;
    SemaphoreHandle_t completion_semaphore;
    void* data;
};

class OOBETask {
   public:
    OOBETask(const char* name) : queue_(xQueueCreate(2, sizeof(TaskMessage))) {
        CHECK(xTaskCreate(StaticTaskFunc, name, configMINIMAL_STACK_SIZE * 30,
                          this, APP_TASK_PRIORITY, &task_) == pdPASS);
        CHECK(queue_);
    }

    void Start() { SendCommandBlocking(kCmdStart); }

    void Stop() { SendCommandBlocking(kCmdStop); }

   protected:
    virtual void TaskFunc() = 0;
    QueueHandle_t queue_;

   private:
    static void StaticTaskFunc(void* param) {
        auto thiz = reinterpret_cast<OOBETask*>(param);
        thiz->TaskFunc();
    }

    void SendCommandBlocking(int command) {
        TaskMessage message = {command, xSemaphoreCreateBinary()};
        CHECK(message.completion_semaphore);
        CHECK(xQueueSend(queue_, &message, portMAX_DELAY) == pdTRUE);
        CHECK(xSemaphoreTake(message.completion_semaphore, portMAX_DELAY) ==
              pdTRUE);
        vSemaphoreDelete(message.completion_semaphore);
    }

    TaskHandle_t task_;
};

class PosenetTask : public OOBETask {
   public:
    PosenetTask() : OOBETask("posenet_task"){};
    void QueueFrame(std::vector<uint8_t>* frame) {
        TaskMessage message = {kCmdProcess, nullptr, frame};
        CHECK(xQueueSend(queue_, &message, portMAX_DELAY) == pdTRUE);
    }

   protected:
    void TaskFunc() override {
        TaskMessage message;
        coral::micro::posenet::Output output;
        while (true) {
            CHECK(xQueueReceive(queue_, &message, portMAX_DELAY) == pdTRUE);

            switch (message.command) {
                case kCmdStart:
                    configASSERT(!started_);
                    started_ = true;
                    printf("Posenet: started\r\n");
                    coral::micro::led::Set(coral::micro::led::LED::kTpu, true);
                    break;
                case kCmdStop:
                    configASSERT(started_);
                    started_ = false;
                    printf("Posenet: stopped\r\n");
                    coral::micro::led::Set(coral::micro::led::LED::kTpu, false);
                    break;
                case kCmdProcess: {
                    configASSERT(started_);
                    auto camera_frame =
                        reinterpret_cast<std::vector<uint8_t>*>(message.data);
                    TfLiteTensor* input = coral::micro::posenet::input();
                    memcpy(tflite::GetTensorData<uint8_t>(input),
                           camera_frame->data(), camera_frame->size());
                    delete camera_frame;

                    coral::micro::posenet::loop(&output, false);
                    int good_poses_count = 0;
                    for (int i = 0; i < coral::micro::posenet::kPoses; ++i) {
                        if (output.poses[i].score > kThreshold) {
                            good_poses_count++;
                        }
                    }
                    if (good_poses_count) {
                        coral::micro::MutexLock lock(posenet_output_mtx);
                        posenet_output =
                            std::make_unique<coral::micro::posenet::Output>(
                                output);
                        posenet_output_time = xTaskGetTickCount();
                    }
                } break;

                default:
                    printf("Unknown command: %d\r\n", message.command);
                    break;
            }

            // Signal the command completion semaphore, if present.
            if (message.completion_semaphore) {
                CHECK(xSemaphoreGive(message.completion_semaphore) == pdTRUE);
            }
        }
    }

   private:
    bool started_ = false;
};

class CameraTask : public OOBETask {
   public:
    CameraTask(PosenetTask* posenet_task)
        : OOBETask("camera_task"), posenet_task_(posenet_task){};

   protected:
    void TaskFunc() override {
        TaskMessage message;
        while (true) {
            CHECK(xQueueReceive(queue_, &message, portMAX_DELAY) == pdTRUE);

            switch (message.command) {
                case kCmdStart:
                    configASSERT(!started_);
                    started_ = true;
                    coral::micro::CameraTask::GetSingleton()->Enable(
                        coral::micro::camera::Mode::STREAMING);
                    printf("Camera: started\r\n");
                    posenet_task_->Start();
                    QueueProcess();
                    break;
                case kCmdStop:
                    configASSERT(started_);
                    coral::micro::CameraTask::GetSingleton()->Disable();
                    started_ = false;
                    printf("Camera: stopped\r\n");
                    posenet_task_->Stop();
                    break;
                case kCmdProcess: {
                    if (!started_) {
                        continue;
                    }

                    std::vector<uint8_t> input(
                        coral::micro::posenet::kPosenetSize);
                    coral::micro::camera::FrameFormat fmt;
                    fmt.width = coral::micro::posenet::kPosenetWidth;
                    fmt.height = coral::micro::posenet::kPosenetHeight;
                    fmt.fmt = coral::micro::camera::Format::RGB;
                    fmt.filter = coral::micro::camera::FilterMethod::BILINEAR;
                    fmt.preserve_ratio = false;
                    fmt.buffer = input.data();
                    coral::micro::CameraTask::GetFrame({fmt});

                    {
                        coral::micro::MutexLock lock(camera_output_mtx);
                        camera_output = std::move(input);
                    }

                    // Signal posenet.
                    posenet_task_->QueueFrame(
                        new std::vector<uint8_t>(camera_output));

                    // Process next camera frame.
                    QueueProcess();
                } break;

                default:
                    printf("Unknown command: %d\r\n", message.command);
                    break;
            }

            // Signal the command completion semaphore, if present.
            if (message.completion_semaphore) {
                CHECK(xSemaphoreGive(message.completion_semaphore) == pdTRUE);
            }
        }
    }

   private:
    void QueueProcess() {
        TaskMessage message = {kCmdProcess};
        CHECK(xQueueSend(queue_, &message, portMAX_DELAY) == pdTRUE);
    }

    PosenetTask* posenet_task_ = nullptr;
    bool started_ = false;
};

#if defined(OOBE_SIMPLE)
void Main() {
    coral::micro::IPCM7::GetSingleton()->StartM4();
    vTaskSuspend(nullptr);
}
#else
void Main() {
    PosenetTask posenet_task;
    CameraTask camera_task(&posenet_task);
    camera_output_mtx = xSemaphoreCreateMutex();
    CHECK(camera_output_mtx);
    posenet_output_mtx = xSemaphoreCreateMutex();
    CHECK(posenet_output_mtx);

// For the OOBE Demo, bring up WiFi and Ethernet. For now these are active
// but unused.
#if defined(OOBE_DEMO_ETHERNET)
    coral::micro::InitializeEthernet(false);
#elif defined(OOBE_DEMO_WIFI)
    coral::micro::TurnOnWiFi();
    if (!coral::micro::ConnectToWiFi()) {
        // If connecting to wi-fi fails, turn our LEDs on solid, and halt.
        coral::micro::led::Set(coral::micro::led::LED::kPower, true);
        coral::micro::led::Set(coral::micro::led::LED::kUser, true);
        vTaskSuspend(nullptr);
    }
#endif  // defined(OOBE_DEMO_ETHERNET)

    coral::micro::HttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    http_server.AddUriHandler(coral::micro::FileSystemUriHandler{});
    coral::micro::UseHttpServer(&http_server);

    coral::micro::IPCM7::GetSingleton()->RegisterAppMessageHandler(
        HandleAppMessage, xTaskGetCurrentTaskHandle());
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(
        coral::micro::PerformanceMode::kMax);
    if (!coral::micro::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }

    coral::micro::IPCM7::GetSingleton()->StartM4();

#if defined(OOBE_DEMO)
    int count = 0;
#endif  // defined (OOBE_DEMO)

    vTaskSuspend(nullptr);
    while (true) {
        printf("CM7 awoken\r\n");

        // Start camera_task processing, which will start posenet_task.
        camera_task.Start();
        posenet_output_time = xTaskGetTickCount();

        while (true) {
// For OOBE Demo, run 20 iterations of this loop - each contains a one
// second delay. For normal OOBE, check that the posenet task hasn't
// progressed for 5 seconds (i.e. no poses detected).
#if defined(OOBE_DEMO)
            if (count >= 20) {
                count = 0;
                break;
            }
            ++count;
            printf("M7 %d\r\n", count);
#else
            TickType_t now = xTaskGetTickCount();
            if ((now - posenet_output_time) > pdMS_TO_TICKS(5000)) {
                break;
            }
#endif  // defined(OOBE_DEMO)

            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        printf("Transition back to M4\r\n");

        // Stop camera_task processing. This will also stop posenet_task.
        camera_task.Stop();

        coral::micro::ipc::Message msg;
        msg.type = coral::micro::ipc::MessageType::APP;
        coral::micro::IPCM7::GetSingleton()->SendMessage(msg);
        vTaskSuspend(nullptr);
    }
}
#endif  // defined(OOBE_SIMPLE)

}  // namespace
}  // namespace coral::micro

extern "C" void app_main(void* param) {
    (void)param;
    coral::micro::Main();
    vTaskSuspend(nullptr);
}
