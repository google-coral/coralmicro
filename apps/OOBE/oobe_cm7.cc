#include "libs/base/httpd.h"
#include "libs/base/ipc_m7.h"
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
#include "libs/base/gpio.h"
#include "libs/base/wifi.h"
#endif  // defined(OOBE_DEMO_WIFI)

#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

namespace valiant {
namespace oobe {
namespace {
constexpr float kThreshold = 0.4;

constexpr int kPosenetWidth = 481;
constexpr int kPosenetHeight = 353;
constexpr int kPosenetDepth = 3;
constexpr int kPosenetSize = kPosenetWidth * kPosenetHeight * kPosenetDepth;
constexpr int kCmdStart = 1;
constexpr int kCmdStop = 2;
constexpr int kCmdProcess = 3;

SemaphoreHandle_t camera_output_mtx;
std::vector<uint8_t> camera_output;

SemaphoreHandle_t posenet_output_mtx;
std::unique_ptr<valiant::posenet::Output> posenet_output;
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

HttpServer::Content UriHandler(const char *name) {
  if (std::strcmp(name, "/camera") == 0) {
    valiant::MutexLock lock(camera_output_mtx);
    if (camera_output.empty()) return {};
    return valiant::HttpServer::Content{std::move(camera_output)};
  }

  if (std::strcmp(name, "/pose") == 0) {
    valiant::MutexLock lock(posenet_output_mtx);
    if (!posenet_output) return {};
    auto json = CreatePoseJson(*posenet_output, kThreshold);
    posenet_output.reset();
    return json;
  }

  return {};
}

void HandleAppMessage(
    const uint8_t data[valiant::ipc::kMessageBufferDataSize], void *param) {
  (void)data;
  vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

struct TaskMessage {
  int command;
  SemaphoreHandle_t completion_semaphore;
  void *data;
};

class OOBETask {
 public:
  OOBETask(const char *name) : queue_(xQueueCreate(2, sizeof(TaskMessage))) {
    xTaskCreate(StaticTaskFunc, name, configMINIMAL_STACK_SIZE * 30, this,
                APP_TASK_PRIORITY, &task_);
  }

  void Start() { SendCommandBlocking(kCmdStart); }

  void Stop() { SendCommandBlocking(kCmdStop); }

 protected:
  virtual void TaskFunc() = 0;
  QueueHandle_t queue_;

 private:
  static void StaticTaskFunc(void *param) {
    auto thiz = reinterpret_cast<OOBETask *>(param);
    thiz->TaskFunc();
  }

  void SendCommandBlocking(int command) {
    TaskMessage message = {command, xSemaphoreCreateBinary()};
    xQueueSend(queue_, &message, portMAX_DELAY);
    xSemaphoreTake(message.completion_semaphore, portMAX_DELAY);
    vSemaphoreDelete(message.completion_semaphore);
  }

  TaskHandle_t task_;
};

class PosenetTask : public OOBETask {
 public:
  PosenetTask() : OOBETask("posenet_task"){};
  void QueueFrame(std::vector<uint8_t> *frame) {
    TaskMessage message = {kCmdProcess, nullptr, frame};
    xQueueSend(queue_, &message, portMAX_DELAY);
  }

 protected:
  void TaskFunc() override {
    TaskMessage message;
    valiant::posenet::Output output;
    while (true) {
      xQueueReceive(queue_, &message, portMAX_DELAY);

      switch (message.command) {
        case kCmdStart:
          configASSERT(!started_);
          started_ = true;
          printf("Posenet: started\r\n");
          break;
        case kCmdStop:
          configASSERT(started_);
          started_ = false;
          printf("Posenet: stopped\r\n");
          break;
        case kCmdProcess: {
          configASSERT(started_);
          auto camera_frame =
              reinterpret_cast<std::vector<uint8_t> *>(message.data);
          TfLiteTensor *input = valiant::posenet::input();
          memcpy(tflite::GetTensorData<uint8_t>(input), camera_frame->data(),
                 camera_frame->size());
          delete camera_frame;

          valiant::posenet::loop(&output, false);
          int good_poses_count = 0;
          for (int i = 0; i < valiant::posenet::kPoses; ++i) {
            if (output.poses[i].score > kThreshold) {
              good_poses_count++;
            }
          }
          if (good_poses_count) {
            valiant::MutexLock lock(posenet_output_mtx);
            posenet_output = std::make_unique<valiant::posenet::Output>(output);
            posenet_output_time = xTaskGetTickCount();
          }
        } break;

        default:
          printf("Unknown command: %d\r\n", message.command);
          break;
      }

      // Signal the command completion semaphore, if present.
      if (message.completion_semaphore) {
        xSemaphoreGive(message.completion_semaphore);
      }
    }
  }

 private:
  bool started_ = false;
};

class CameraTask : public OOBETask {
 public:
  CameraTask(PosenetTask *posenet_task)
      : OOBETask("camera_task"), posenet_task_(posenet_task){};

 protected:
  void TaskFunc() override {
    TaskMessage message;
    while (true) {
      xQueueReceive(queue_, &message, portMAX_DELAY);

      switch (message.command) {
        case kCmdStart:
          configASSERT(!started_);
          started_ = true;
          valiant::CameraTask::GetSingleton()->Enable(
              valiant::camera::Mode::STREAMING);
          printf("Camera: started\r\n");
          posenet_task_->Start();
          QueueProcess();
          break;
        case kCmdStop:
          configASSERT(started_);
          valiant::CameraTask::GetSingleton()->Disable();
          started_ = false;
          printf("Camera: stopped\r\n");
          posenet_task_->Stop();
          break;
        case kCmdProcess: {
          if (!started_) {
            continue;
          }

          std::vector<uint8_t> input(kPosenetSize);
          valiant::camera::FrameFormat fmt;
          fmt.width = kPosenetWidth;
          fmt.height = kPosenetHeight;
          fmt.fmt = valiant::camera::Format::RGB;
          fmt.preserve_ratio = false;
          fmt.buffer = input.data();
          valiant::CameraTask::GetFrame({fmt});

          {
            valiant::MutexLock lock(camera_output_mtx);
            camera_output = std::move(input);
          }

          // Signal posenet.
          posenet_task_->QueueFrame(new std::vector<uint8_t>(camera_output));

          // Process next camera frame.
          QueueProcess();
        } break;

        default:
          printf("Unknown command: %d\r\n", message.command);
          break;
      }

      // Signal the command completion semaphore, if present.
      if (message.completion_semaphore) {
        xSemaphoreGive(message.completion_semaphore);
      }
    }
  }

 private:
  void QueueProcess() {
    TaskMessage message = {kCmdProcess};
    xQueueSend(queue_, &message, portMAX_DELAY);
  }

  PosenetTask *posenet_task_ = nullptr;
  bool started_ = false;
};

void Main() {
  PosenetTask posenet_task;
  CameraTask camera_task(&posenet_task);
  camera_output_mtx = xSemaphoreCreateMutex();
  posenet_output_mtx = xSemaphoreCreateMutex();

// For the OOBE Demo, bring up WiFi and Ethernet. For now these are active
// but unused.
#if defined(OOBE_DEMO_ETHERNET)
  valiant::InitializeEthernet(false);
#elif defined(OOBE_DEMO_WIFI)
  valiant::TurnOnWiFi();
  if (!valiant::ConnectToWiFi()) {
    // If connecting to wi-fi fails, turn our LEDs on solid, and halt.
    valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, true);
    valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, true);
    valiant::gpio::SetGpio(valiant::gpio::Gpio::kTpuLED, true);
    vTaskSuspend(nullptr);
  }
#endif  // defined(OOBE_DEMO_ETHERNET)

  valiant::HttpServer http_server;
  http_server.AddUriHandler(UriHandler);
  http_server.AddUriHandler(valiant::FileSystemUriHandler{});
  valiant::UseHttpServer(&http_server);

  valiant::IPCM7::GetSingleton()->RegisterAppMessageHandler(
      HandleAppMessage, xTaskGetCurrentTaskHandle());
  valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
  valiant::EdgeTpuManager::GetSingleton()->OpenDevice(
      valiant::PerformanceMode::kMax);
  if (!valiant::posenet::setup()) {
    printf("setup() failed\r\n");
    vTaskSuspend(nullptr);
  }

  valiant::IPCM7::GetSingleton()->StartM4();

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

    valiant::ipc::Message msg;
    msg.type = valiant::ipc::MessageType::APP;
    valiant::IPCM7::GetSingleton()->SendMessage(msg);
    vTaskSuspend(nullptr);
  }
}
}  // namespace
}  // namespace oobe
}  // namespace valiant

extern "C" void app_main(void *param) {
  (void)param;
  valiant::oobe::Main();
  vTaskSuspend(nullptr);
}
