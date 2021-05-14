#include "libs/base/ipc_m7.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include <cstdio>
#include <cstring>

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void* param) {
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

extern "C" void app_main(void *param) {
    valiant::IPCM7::GetSingleton()->RegisterAppMessageHandler(HandleAppMessage, xTaskGetCurrentTaskHandle());
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }
    static_cast<valiant::IPCM7*>(valiant::IPC::GetSingleton())->StartM4();
    vTaskSuspend(NULL);
    while (true) {
        printf("CM7 awoken\r\n");

        valiant::posenet::Output output;
        valiant::CameraTask::GetSingleton()->Enable();

        TickType_t last_good_pose = xTaskGetTickCount();
        while (true) {
            TfLiteTensor *input = valiant::posenet::input();
            valiant::camera::FrameFormat fmt;
            fmt.width = input->dims->data[2];
            fmt.height = input->dims->data[1];
            fmt.fmt = valiant::camera::Format::RGB;
            fmt.preserve_ratio = false;
            valiant::CameraTask::GetFrame(fmt, tflite::GetTensorData<uint8_t>(input));
            valiant::posenet::loop(&output);

            bool good_pose = false;
            for (int i = 0; i < valiant::posenet::kPoses; ++i) {
                if (output.poses[i].score > 0.4) {
                    good_pose = true;
                    break;
                }
            }

            TickType_t now = xTaskGetTickCount();
            if (good_pose) {
                last_good_pose = now;
            }

            if ((now - last_good_pose) > pdMS_TO_TICKS(5000)) {
                break;
            }
        }

        printf("Transition back to M4\r\n");
        valiant::CameraTask::GetSingleton()->Disable();
        valiant::ipc::Message msg;
        msg.type = valiant::ipc::MessageType::APP;
        valiant::IPCM7::GetSingleton()->SendMessage(msg);
        vTaskSuspend(NULL);
    }
}
