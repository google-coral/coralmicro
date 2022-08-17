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

#include <cstdio>

#include "libs/base/filesystem.h"
#include "libs/base/ipc_m4.h"
#include "libs/base/led.h"
#include "libs/base/main_freertos_m4.h"
#include "libs/camera/camera.h"
#include "libs/tensorflow/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/freertos_kernel/include/timers.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/schema/schema_generated.h"

namespace coralmicro {
namespace {
// In the demo version, we use this variable to signal switching back to the M7,
// This signal is set to true every 5 second.
#if defined(MULTICORE_MODEL_CASCADE_DEMO)
bool volatile g_switch_to_m7_signal = false;
#endif

// Defines person detection model attributes.
constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kPersonIndex = 1;
constexpr int kNotAPersonIndex = 0;

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 136 * 1024;
STATIC_TENSOR_ARENA_IN_OCRAM(tensor_arena, kTensorArenaSize);
}  // namespace

bool DetectPerson(tflite::MicroInterpreter* interpreter) {
  auto* input_tensor = interpreter->input(0);

  // Get image and copy it into the input tensor.
  coralmicro::CameraFrameFormat fmt{CameraFormat::kY8,
                                    coralmicro::CameraFilterMethod::kBilinear,
                                    CameraRotation::k270,
                                    kNumCols,
                                    kNumRows,
                                    /*preserve_ration=*/false,
                                    tflite::GetTensorData<uint8>(input_tensor)};
  if (!coralmicro::CameraTask::GetSingleton()->GetFrame({fmt})) {
    printf("Image capture failed\r\n");
    vTaskSuspend(nullptr);
  }

  // Run the model on this input and make sure it succeeds.
  if (interpreter->Invoke() != kTfLiteOk) {
    printf("Invoke failed\r\n");
    vTaskSuspend(nullptr);
  }

  auto* output_tensor = interpreter->output(0);
  // Process the inference results.
  auto person_score =
      static_cast<int8_t>(output_tensor->data.uint8[kPersonIndex]);
  auto no_person_score =
      static_cast<int8_t>(output_tensor->data.uint8[kNotAPersonIndex]);
  printf("person_score: %d no_person_score: %d\r\n", person_score,
         no_person_score);
  return person_score > no_person_score;
}

[[noreturn]] void Main() {
  // This handler resume this m4 task, as soon as signal from m7 is received.
  IpcM4::GetSingleton()->RegisterAppMessageHandler(
      [handle = xTaskGetCurrentTaskHandle()](const uint8_t[]) {
        vTaskResume(handle);
      });
  CameraTask::GetSingleton()->Init(I2C5Handle());
  CameraTask::GetSingleton()->SetPower(false);
  vTaskDelay(pdMS_TO_TICKS(100));
  CameraTask::GetSingleton()->SetPower(true);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  std::vector<uint8_t> person_detect_model_data;
  if (!coralmicro::LfsReadFile("/models/person_detect_model.tflite",
                               &person_detect_model_data)) {
    printf("Error: Cannot load model\r\n");
    vTaskSuspend(nullptr);
  }
  auto model = tflite::GetModel(person_detect_model_data.data());
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf(
        "Model provided is schema version %lu not equal to supported version "
        "%d\r\n",
        model->version(), TFLITE_SCHEMA_VERSION);
    vTaskSuspend(nullptr);
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  tflite::MicroMutableOpResolver<5> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  tflite::MicroErrorReporter error_reporter;
  tflite::MicroInterpreter interpreter(model, micro_op_resolver, tensor_arena,
                                       kTensorArenaSize, &error_reporter);

  // Allocate memory from the tensor_arena for the model's tensors.
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("AllocateTensors() failed\r\n");
    vTaskSuspend(nullptr);
  }

  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);

#if defined(MULTICORE_MODEL_CASCADE_DEMO)
  // For the demo version, this timer signals to switch to the m7 every 5
  // seconds.
  TimerHandle_t m4_timer = xTimerCreate(
      "m4_timer", pdMS_TO_TICKS(10000), pdTRUE, nullptr,
      +[](TimerHandle_t xTimer) { g_switch_to_m7_signal = true; });
  xTimerStart(m4_timer, 0);
#endif  // defined(MULTICORE_MODEL_CASCADE_DEMO)

  while (true) {
    printf("M4 Started, detecting person...\r\n");
    CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);

    // For the demo version, ensures that the switch to m7 signal is always
    // false until the timer let us switch back to the M7.
#if defined(MULTICORE_MODEL_CASCADE_DEMO)
    g_switch_to_m7_signal = false;
#endif

    while (true) {
      bool person_detected = DetectPerson(&interpreter);
      // Turn on user led if a person is detected, or turn it off if no person
      // detected.
      LedSet(Led::kUser, person_detected);

      // For demo version, switch to m7 when the signal is set by the timer, for
      // the normal version, switch as soon as a person is detected.
#if defined(MULTICORE_MODEL_CASCADE_DEMO)
      if (g_switch_to_m7_signal) {
        break;
      }
#else
      if (person_detected) {
        break;
      }
#endif  // !defined(MULTICORE_MODEL_CASCADE_DEMO)
    }
    printf("Person detected, let M7 take over.\r\n");
    CameraTask::GetSingleton()->Disable();
    IpcMessage msg{};
    msg.type = IpcMessageType::kApp;
    IpcM4::GetSingleton()->SendMessage(msg);
    printf("M4 stopped\r\n");
    vTaskSuspend(nullptr);
  }
}
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
}
