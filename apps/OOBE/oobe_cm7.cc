#include "apps/OOBE/jpeg.h"
#include "libs/base/http_server_handlers.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "libs/base/network.h"
#include "libs/base/reset.h"
#include "libs/base/strings.h"
#include "libs/base/watchdog.h"
#include "libs/posenet/posenet.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tasks/CameraTask/camera_task.h"
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

#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace coral::micro {
namespace {
constexpr float kThreshold = 0.4;

constexpr int kCmdStart = 1;
constexpr int kCmdStop = 2;
constexpr int kCmdProcess = 3;

constexpr int kNetworkPort = 27000;

constexpr int kMessageTypeSetup = 0;
constexpr int kMessageTypeImageData = 1;
constexpr int kMessageTypePoseData = 2;
constexpr int kLowPowerChange = 3;

template <typename T>
constexpr uint8_t byte(T value, int i) {
    return static_cast<uint8_t>(value >> 8 * i);
}

void WriteMessageImageInfo(int fd) {
    constexpr auto w = posenet::kPosenetWidth;
    constexpr auto h = posenet::kPosenetHeight;
    constexpr uint8_t msg[] = {byte(w, 0), byte(w, 1), byte(w, 2), byte(w, 3),
                               byte(h, 0), byte(h, 1), byte(h, 2), byte(h, 3)};
    WriteMessage(fd, kMessageTypeSetup, msg, sizeof(msg));
}

struct TaskMessage {
    int command;
    SemaphoreHandle_t completion_semaphore;
    void* data;
};

int CountPoses(const posenet::Output& output, float threshold) {
    int count = 0;
    for (int i = 0; i < output.num_poses; ++i)
        if (output.poses[i].score >= threshold) ++count;
    return count;
}

void CreatePoseJson(const posenet::Output& output, float threshold,
                    std::vector<uint8_t>* json) {
    CHECK(json);
    json->clear();
    int num_appended_poses = 0;
    StrAppend(json, "[");
    for (int i = 0; i < output.num_poses; ++i) {
        if (output.poses[i].score < threshold) continue;

        StrAppend(json, num_appended_poses != 0 ? ",\n" : "");
        StrAppend(json, "{\n");
        StrAppend(json, "  \"score\": %g,\n", output.poses[i].score);
        StrAppend(json, "  \"keypoints\": [\n");
        for (int j = 0; j < posenet::kKeypoints; ++j) {
            const auto& kp = output.poses[i].keypoints[j];
            StrAppend(json, "    [%g, %g, %g]", kp.score, kp.x, kp.y);
            StrAppend(json, j != posenet::kKeypoints - 1 ? ",\n" : "\n");
        }
        StrAppend(json, "  ]\n");
        StrAppend(json, "}");
        ++num_appended_poses;
    }
    StrAppend(json, "]");
}

template <typename Derived>
class Task {
   public:
    Task(const char* name, int priority) {
        CHECK(xTaskCreate(StaticRun, name, configMINIMAL_STACK_SIZE * 30, this,
                          priority, &task_) == pdPASS);
    }

   private:
    static void StaticRun(void* param) {
        static_cast<Derived*>(param)->Run();
        vTaskSuspend(nullptr);
    }
    TaskHandle_t task_;
};

class NetworkTask : private Task<NetworkTask> {
   public:
    NetworkTask()
        : Task("oobe_network_task", APP_TASK_PRIORITY),
          mutex_(xSemaphoreCreateMutex()),
          last_pose_data_(xTaskGetTickCount()) {
        CHECK(mutex_);
    }

    void Send(uint8_t type, const void* bytes, size_t size) {
        MutexLock lock(mutex_);

        if (type == kMessageTypePoseData) last_pose_data_ = xTaskGetTickCount();

        if (client_socket_ == -1) return;

        if (SocketHasPendingInput(client_socket_)) {
            ResetClientSocket();
            return;
        }

        if (WriteMessage(client_socket_, type, bytes, size) != IOStatus::kOk)
            ResetClientSocket();
    }

    bool PosenetInactiveForMs(int ms) const {
        MutexLock lock(mutex_);
        return xTaskGetTickCount() - last_pose_data_ > pdMS_TO_TICKS(ms);
    }

    void ResetPosenetTimer() {
        MutexLock lock(mutex_);
        last_pose_data_ = xTaskGetTickCount();
    }

    void Run() {
        int server_socket = SocketServer(kNetworkPort, /*backlog=*/5);
        while (true) {
            printf("INFO: Waiting for clients on %d...\r\n", kNetworkPort);
            const int client_socket = SocketAccept(server_socket);
            if (client_socket == -1) {
                printf("ERROR: Cannot accept client.\r\n");
                continue;
            }

            {
                MutexLock lock(mutex_);
                ResetClientSocket(client_socket);
                printf("INFO: Client #%d connected.\r\n", client_socket);
                WriteMessageImageInfo(client_socket);
            }
        }
    }

   private:
    SemaphoreHandle_t mutex_;
    TickType_t last_pose_data_ = 0;
    int client_socket_ = -1;

    void ResetClientSocket(int sockfd = -1) {
        if (client_socket_ != -1) SocketClose(client_socket_);
        client_socket_ = sockfd;
    }
};

class PosenetTask : private Task<PosenetTask> {
   public:
    PosenetTask(NetworkTask* network_task_)
        : Task("oobe_posenet_task", APP_TASK_PRIORITY),
          network_task_(network_task_),
          queue_(xQueueCreate(1, sizeof(char))) {
        CHECK(queue_);
    }

    void Put(const std::vector<uint8_t>& frame) {
        if (uxQueueMessagesWaiting(queue_) == 0) {
            assert(frame.size() == posenet::kPosenetSize);
            std::memcpy(tflite::GetTensorData<uint8_t>(posenet::input()),
                        frame.data(), posenet::kPosenetSize);
            char cmd = 0;
            CHECK(xQueueSendToBack(queue_, &cmd, portMAX_DELAY) == pdTRUE);
        }
    }

    void Run() const {
        posenet::Output output;
        std::vector<uint8_t> json;
        json.reserve(2048);  // Assume JSON size around 2K.
        while (true) {
            char cmd;
            CHECK(xQueuePeek(queue_, &cmd, portMAX_DELAY) == pdTRUE);
            posenet::loop(&output, false);

            if (CountPoses(output, kThreshold) > 0) {
                CreatePoseJson(output, kThreshold, &json);
                network_task_->Send(kMessageTypePoseData, json.data(),
                                    json.size());
            }

            CHECK(xQueueReceive(queue_, &cmd, portMAX_DELAY) == pdTRUE);
        }
    }

   private:
    NetworkTask* network_task_;
    QueueHandle_t queue_;
};

class CameraTask : private Task<CameraTask> {
   public:
    CameraTask(NetworkTask* network_task, PosenetTask* posenet_task)
        : Task("oobe_camera_task", APP_TASK_PRIORITY),
          network_task_(network_task),
          posenet_task_(posenet_task),
          queue_(xQueueCreate(2, sizeof(TaskMessage))) {
        CHECK(queue_);
    };

    void Start() { SendCommandBlocking(kCmdStart); }

    void Stop() { SendCommandBlocking(kCmdStop); }

    void Run() const {
        bool started = false;

        std::vector<uint8_t> input(posenet::kPosenetSize);
        std::vector<unsigned char> jpeg(1024 * 70);

        TaskMessage message;
        while (true) {
            CHECK(xQueueReceive(queue_, &message, portMAX_DELAY) == pdTRUE);

            switch (message.command) {
                case kCmdStart:
                    configASSERT(!started);
                    started = true;
                    coral::micro::CameraTask::GetSingleton()->Enable(
                        camera::Mode::STREAMING);
                    printf("Camera: started\r\n");
                    QueueProcess();
                    break;
                case kCmdStop:
                    configASSERT(started);
                    coral::micro::CameraTask::GetSingleton()->Disable();
                    started = false;
                    printf("Camera: stopped\r\n");
                    break;
                case kCmdProcess: {
                    if (!started) {
                        continue;
                    }

                    coral::micro::camera::FrameFormat fmt;
                    fmt.width = posenet::kPosenetWidth;
                    fmt.height = posenet::kPosenetHeight;
                    fmt.fmt = camera::Format::RGB;
                    fmt.filter = camera::FilterMethod::BILINEAR;
                    fmt.preserve_ratio = false;
                    fmt.buffer = input.data();
                    coral::micro::CameraTask::GetFrame({fmt});

                    auto jpeg_size = JpegCompressRgb(
                        input.data(), fmt.width, fmt.height,
                        /*quality=*/75, jpeg.data(), jpeg.size());
                    network_task_->Send(kMessageTypeImageData, jpeg.data(),
                                        jpeg_size);

                    posenet_task_->Put(input);

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
    void SendCommandBlocking(int command) {
        TaskMessage message = {command, xSemaphoreCreateBinary()};
        CHECK(message.completion_semaphore);
        CHECK(xQueueSend(queue_, &message, portMAX_DELAY) == pdTRUE);
        CHECK(xSemaphoreTake(message.completion_semaphore, portMAX_DELAY) ==
              pdTRUE);
        vSemaphoreDelete(message.completion_semaphore);
    }

    void QueueProcess() const {
        TaskMessage message = {kCmdProcess};
        CHECK(xQueueSend(queue_, &message, portMAX_DELAY) == pdTRUE);
    }

    NetworkTask* network_task_ = nullptr;
    PosenetTask* posenet_task_ = nullptr;
    QueueHandle_t queue_;
};

void reset_count_rpc(struct jsonrpc_request* r) {
    const auto reset_stats = GetResetStats();
    jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %d}", "watchdog_resets",
                           reset_stats.watchdog_resets, "lockup_resets",
                           reset_stats.lockup_resets, "reset_register",
                           reset_stats.reset_reason);
}

void HandleAppMessage(const uint8_t data[ipc::kMessageBufferDataSize],
                      void* param) {
    (void)data;
    vTaskResume(static_cast<TaskHandle_t>(param));
}

void Main() {
    constexpr watchdog::WatchdogConfig wdt_config = {
        .timeout_s = 8,
        .pet_rate_s = 3,
        .enable_irq = false,
    };
    watchdog::StartWatchdog(wdt_config);

    NetworkTask network_task;
    PosenetTask posenet_task(&network_task);
    CameraTask camera_task(&network_task, &posenet_task);

// For the OOBE Demo, bring up WiFi and Ethernet. For now these are active
// but unused.
#if defined(OOBE_DEMO_ETHERNET)
    InitializeEthernet(false);
#elif defined(OOBE_DEMO_WIFI)
    TurnOnWiFi();
    if (!ConnectWiFi()) {
        // If connecting to wi-fi fails, turn our LEDs on solid, and halt.
        led::Set(led::LED::kPower, true);
        led::Set(led::LED::kUser, true);
        vTaskSuspend(nullptr);
    }
#endif  // defined(OOBE_DEMO_ETHERNET)

    JsonRpcHttpServer http_server;
    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export("reset_count", reset_count_rpc);
    http_server.AddUriHandler(TaskStatsUriHandler{});
    http_server.AddUriHandler(FileSystemUriHandler{});
    UseHttpServer(&http_server);

    IPCM7::GetSingleton()->RegisterAppMessageHandler(
        HandleAppMessage, xTaskGetCurrentTaskHandle());
    auto tpu_context =
        EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
    if (!posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    IPCM7::GetSingleton()->StartM4();

#if defined(OOBE_DEMO)
    int count = 0;
#endif  // defined (OOBE_DEMO)

    bool low_power = true;
    vTaskSuspend(nullptr);
    while (true) {
        printf("CM7 awoken\r\n");
        low_power = false;
        network_task.Send(kLowPowerChange, &low_power, 1);
        network_task.ResetPosenetTimer();

        // Start camera_task processing, which will start posenet_task.
        camera_task.Start();

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
            if (network_task.PosenetInactiveForMs(5000)) break;
#endif  // defined(OOBE_DEMO)

            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        printf("Transition back to M4\r\n");
        low_power = true;
        network_task.Send(kLowPowerChange, &low_power, 1);

        // Stop camera_task processing. This will also stop posenet_task.
        camera_task.Stop();

        ipc::Message msg;
        msg.type = ipc::MessageType::APP;
        IPCM7::GetSingleton()->SendMessage(msg);
        vTaskSuspend(nullptr);
    }
}
}  // namespace
}  // namespace coral::micro

extern "C" void app_main(void* param) {
    (void)param;
    coral::micro::Main();
    vTaskSuspend(nullptr);
}
