// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libs/base/filesystem.h"
#include "libs/base/http_server_handlers.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/led.h"
#include "libs/base/mutex.h"
#include "libs/base/network.h"
#include "libs/base/reset.h"
#include "libs/base/strings.h"
#include "libs/base/watchdog.h"
#include "libs/camera/camera.h"
#include "libs/libjpeg/jpeg.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tensorflow/posenet.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

#if defined(MULTICORE_MODEL_CASCADE_ETHERNET)
#include "libs/base/ethernet.h"
#endif  // defined(MULTICORE_MODEL_CASCADE_ETHERNET)

#if defined(MULTICORE_MODEL_CASCADE_WIFI)
#include "libs/base/wifi.h"
#endif  // defined(MULTICORE_MODEL_CASCADE_WIFI)

#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace coralmicro {
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

constexpr int kTensorArenaSize = 1024 * 1024 * 2;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);
constexpr char kModelPath[] =
    "/models/"
    "posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite";
constexpr int kModelWidth = 324;
constexpr int kModelHeight = 324;
constexpr int kModelSize = kModelWidth * kModelHeight * /*depth*/ 3;

constexpr int kLogInterval = 15;

template <typename T>
constexpr uint8_t byte(T value, int i) {
  return static_cast<uint8_t>(value >> 8 * i);
}

void WriteMessageImageInfo(int fd) {
  constexpr uint8_t msg[] = {byte(kModelWidth, 0),  byte(kModelWidth, 1),
                             byte(kModelWidth, 2),  byte(kModelWidth, 3),
                             byte(kModelHeight, 0), byte(kModelHeight, 1),
                             byte(kModelHeight, 2), byte(kModelHeight, 3)};
  WriteMessage(fd, kMessageTypeSetup, msg, sizeof(msg));
}

struct TaskMessage {
  int command;
  SemaphoreHandle_t completion_semaphore;
  void* data;
};

void CreatePoseJson(const std::vector<tensorflow::Pose>& poses, float threshold,
                    std::vector<uint8_t>* json) {
  CHECK(json);
  json->clear();
  int num_appended_poses = 0;
  StrAppend(json, "[");
  for (const auto& pose : poses) {
    if (pose.score < threshold) continue;

    StrAppend(json, num_appended_poses != 0 ? ",\n" : "");
    StrAppend(json, "{\n");
    StrAppend(json, "  \"score\": %g,\n", pose.score);
    StrAppend(json, "  \"keypoints\": [\n");
    for (int j = 0; j < tensorflow::kKeypoints; ++j) {
      const auto& kp = pose.keypoints[j];
      StrAppend(json, "    [%g, %g, %g]", kp.score, kp.x, kp.y);
      StrAppend(json, j != tensorflow::kKeypoints - 1 ? ",\n" : "\n");
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
      : Task("cascade_network_task", kAppTaskPriority),
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

  [[nodiscard]] bool PosenetInactiveForMs(int ms) const {
    MutexLock lock(mutex_);
    return xTaskGetTickCount() - last_pose_data_ > pdMS_TO_TICKS(ms);
  }

  void ResetPosenetTimer() {
    MutexLock lock(mutex_);
    last_pose_data_ = xTaskGetTickCount();
  }

  [[noreturn]] void Run() {
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
  explicit PosenetTask(NetworkTask* network_task_,
                       std::shared_ptr<tflite::MicroInterpreter> interpreter)
      : Task("cascade_posenet_task", kAppTaskPriority),
        network_task_(network_task_),
        queue_(xQueueCreate(1, sizeof(char))),
        interpreter_(std::move(interpreter)) {
    CHECK(queue_);
    printf("Posenet task started\r\n");
  }

  void Put(const std::vector<uint8_t>& frame) {
    if (uxQueueMessagesWaiting(queue_) == 0) {
      CHECK(frame.size() == kModelSize);
      std::memcpy(tflite::GetTensorData<uint8_t>(interpreter_->input(0)),
                  frame.data(), kModelSize);
      char cmd = 0;
      CHECK(xQueueSendToBack(queue_, &cmd, portMAX_DELAY) == pdTRUE);
    }
  }

  [[noreturn]] void Run() {
    std::vector<uint8_t> json;
    json.reserve(2048);  // Assume JSON size around 2K.
    while (true) {
      char cmd;
      CHECK(xQueuePeek(queue_, &cmd, portMAX_DELAY) == pdTRUE);
      counter_++;
      CHECK(interpreter_->Invoke() == kTfLiteOk);
      auto poses = tensorflow::GetPosenetOutput(interpreter_.get(), kThreshold);
      if (!poses.empty()) {
        CreatePoseJson(poses, kThreshold, &json);
        network_task_->Send(kMessageTypePoseData, json.data(), json.size());
      }
      if (poses.size() != num_poses_ || counter_ % kLogInterval == 0) {
          printf("Poses: %u\r\n", poses.size());
      }
      num_poses_ = poses.size();
      CHECK(xQueueReceive(queue_, &cmd, portMAX_DELAY) == pdTRUE);
    }
  }

 private:
  NetworkTask* network_task_;
  QueueHandle_t queue_;
  std::shared_ptr<tflite::MicroInterpreter> interpreter_;
  size_t counter_;
  size_t num_poses_;
};

class MainTask : private Task<MainTask> {
 public:
  MainTask(NetworkTask* network_task, PosenetTask* posenet_task)
      : Task("cascade_camera_task", kAppTaskPriority),
        network_task_(network_task),
        posenet_task_(posenet_task),
        queue_(xQueueCreate(2, sizeof(TaskMessage))) {
    CHECK(queue_);
  };

  void Start() { SendCommandBlocking(kCmdStart); }

  void Stop() { SendCommandBlocking(kCmdStop); }

  void Run() const {
    bool started = false;

    std::vector<uint8_t> input(kModelSize);
    std::vector<unsigned char> jpeg(1024 * 70);

    TaskMessage message{};
    std::optional<std::string> our_ip_addr;
#if defined(MULTICORE_MODEL_CASCADE_ETHERNET)
    our_ip_addr = EthernetGetIp();
#elif defined(MULTICORE_MODEL_CASCADE_WIFI)
    our_ip_addr = WiFiGetIp();
#endif
    while (true) {
      CHECK(xQueueReceive(queue_, &message, portMAX_DELAY) == pdTRUE);

      switch (message.command) {
        case kCmdStart:
          configASSERT(!started);
          started = true;
          CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);
          printf("M7 Main Task: started\r\n");
          if (our_ip_addr.has_value()) {
            printf("My IP address is %s\r\n", our_ip_addr.value().c_str());
          }
          QueueProcess();
          break;
        case kCmdStop:
          configASSERT(started);
          CameraTask::GetSingleton()->Disable();
          started = false;
          printf("M7 Main Task: stopped\r\n");
          break;
        case kCmdProcess: {
          if (!started) {
            continue;
          }

          coralmicro::CameraFrameFormat fmt;
          fmt.width = kModelWidth;
          fmt.height = kModelHeight;
          fmt.fmt = CameraFormat::kRgb;
          fmt.filter = CameraFilterMethod::kBilinear;
          fmt.preserve_ratio = false;
          fmt.buffer = input.data();
          CameraTask::GetSingleton()->GetFrame({fmt});

          auto jpeg_size =
              JpegCompressRgb(input.data(), fmt.width, fmt.height,
                              /*quality=*/75, jpeg.data(), jpeg.size());
          network_task_->Send(kMessageTypeImageData, jpeg.data(), jpeg_size);

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
  const auto reset_stats = ResetGetStats();
  jsonrpc_return_success(r, "{%Q: %d, %Q: %d, %Q: %d}", "watchdog_resets",
                         reset_stats.watchdog_resets, "lockup_resets",
                         reset_stats.lockup_resets, "reset_register",
                         reset_stats.reset_reason);
}

[[noreturn]] void Main() {
  printf("Multicore Model Cascade!\r\n");

  constexpr WatchdogConfig wdt_config = {
      .timeout_s = 8,
      .pet_rate_s = 3,
      .enable_irq = false,
  };
  WatchdogStart(wdt_config);

  auto tpu_context =
      EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
  if (!tpu_context) {
    printf("Failed to get tpu context.\r\n");
    vTaskSuspend(nullptr);
  }
  std::vector<uint8_t> posenet_tflite;
  if (!LfsReadFile(kModelPath, &posenet_tflite)) {
    printf("ERROR: Failed to read model: %s\r\n", kModelPath);
    vTaskSuspend(nullptr);
  }

  // Starts the posenet engine.
  tflite::MicroErrorReporter error_reporter;
  tflite::MicroMutableOpResolver<2> resolver;
  resolver.AddCustom(kCustomOp, RegisterCustomOp());
  resolver.AddCustom(kPosenetDecoderOp, RegisterPosenetDecoderOp());
  auto interpreter = std::make_shared<tflite::MicroInterpreter>(
      tflite::GetModel(posenet_tflite.data()), resolver, tensor_arena,
      kTensorArenaSize, &error_reporter);
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    printf("Failed to allocate tensor\r\n");
    vTaskSuspend(nullptr);
  }

#if defined(MULTICORE_MODEL_CASCADE_ETHERNET)
  if (!EthernetInit(/*default_iface=*/false)) {
    // If initializing Ethernet fails, turn our LEDs on solid, and halt.
    LedSet(Led::kStatus, true);
    LedSet(Led::kUser, true);
    vTaskSuspend(nullptr);
  }
#elif defined(MULTICORE_MODEL_CASCADE_WIFI)
  WiFiTurnOn(/*default_iface=*/false);
  if (!WiFiConnect()) {
    // If connecting to wi-fi fails, turn our LEDs on solid, and halt.
    LedSet(Led::kStatus, true);
    LedSet(Led::kUser, true);
    vTaskSuspend(nullptr);
  }
#endif  // defined(MULTICORE_MODEL_CASCADE_ETHERNET)

  // Starts all tasks.
  NetworkTask network_task;
  PosenetTask posenet_task(&network_task, interpreter);
  MainTask main_task(&network_task, &posenet_task);

  JsonRpcHttpServer http_server;
  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export("reset_count", reset_count_rpc);
  http_server.AddUriHandler(TaskStatsUriHandler{});
  http_server.AddUriHandler(FileSystemUriHandler{});
  UseHttpServer(&http_server);

  // This handler resume this m7 task, as soon as signal from m4 is received.
  IpcM7::GetSingleton()->RegisterAppMessageHandler(
      [handle = xTaskGetCurrentTaskHandle()](const uint8_t[]) {
        vTaskResume(handle);
      });

  IpcM7::GetSingleton()->StartM4();

#if defined(MULTICORE_MODEL_CASCADE_DEMO)
  int count = 0;
#endif  // defined (MULTICORE_MODEL_CASCADE_DEMO)

  bool low_power = true;
  vTaskSuspend(nullptr);
  while (true) {
    low_power = false;
    network_task.Send(kLowPowerChange, &low_power, 1);
    network_task.ResetPosenetTimer();

    // Start camera_task processing, which will start posenet_task.
    main_task.Start();

    while (true) {
// For the demo, run 20 iterations of this loop - each contains a one-second
// delay. For normal operation, check that the posenet task hasn't progressed
// for 5 seconds (i.e. no poses detected).
#if defined(MULTICORE_MODEL_CASCADE_DEMO)
      if (count >= 20) {
        printf("Transitioning back to M4\r\n");
        count = 0;
        break;
      }
      ++count;
      printf("M7 %d\r\n", count);
#else
      if (network_task.PosenetInactiveForMs(5000)) {
        printf("No poses detected for 5 seconds\r\n");
        break;
      }
#endif  // defined(MULTICORE_MODEL_CASCADE_DEMO)
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    low_power = true;
    network_task.Send(kLowPowerChange, &low_power, 1);

    // Stop camera_task processing. This will also stop posenet_task.
    main_task.Stop();

    IpcMessage msg{};
    msg.type = IpcMessageType::kApp;
    IpcM7::GetSingleton()->SendMessage(msg);
    vTaskSuspend(nullptr);
  }
}
}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
