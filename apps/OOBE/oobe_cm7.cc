#include "apps/OOBE/oobe_json.h"

#include "libs/base/httpd.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/base64.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"

#include <cstdio>
#include <cstring>
#include <list>

static bool pause_processing = false;

constexpr float kThreshold = 0.4;

constexpr int kPosenetWidth = 481;
constexpr int kPosenetHeight = 353;
constexpr int kPosenetDepth = 3;
constexpr int kPosenetSize = kPosenetWidth * kPosenetHeight * kPosenetDepth;

static SemaphoreHandle_t posenet_input_mtx;
std::unique_ptr<uint8_t[]> posenet_input;

static SemaphoreHandle_t camera_output_mtx;
std::unique_ptr<uint8_t[]> camera_output;

static SemaphoreHandle_t posenet_output_mtx;
std::unique_ptr<valiant::posenet::Output> posenet_output;
TickType_t posenet_output_time;

constexpr unsigned int kCameraPextension = 0xdeadbeefU;
constexpr unsigned int kPosePextension = 0xbadbeeefU;

static int camera_http_fs_open_custom(void *context, struct fs_file *file, const char *name) {
    static_cast<void>(context);
    constexpr const char *camera = "/camera";
    constexpr const char *pose = "/pose";
    if (strncmp(name, camera, strlen(camera)) == 0) {
        valiant::MutexLock lock(camera_output_mtx);
        file->data = reinterpret_cast<const char*>(camera_output.release());
        file->len = kPosenetSize;
        file->index = kPosenetSize;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        file->is_custom_file = true;
        file->pextension = reinterpret_cast<void*>(kCameraPextension);
        return 1;
    }
    if (strncmp(name, pose, strlen(pose)) == 0) {
        valiant::MutexLock lock(posenet_output_mtx);
        if (!posenet_output) {
            return 0;
        }
        auto pose_json = valiant::oobe::CreatePoseJSON(*posenet_output, kThreshold);
        posenet_output.reset(nullptr);
        file->data = reinterpret_cast<const char*>(pose_json.release());
        file->len = strlen(file->data);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        file->is_custom_file = true;
        file->pextension = reinterpret_cast<void*>(kPosePextension);
        return 1;
    }
    return 0;
}

static void camera_http_fs_close_custom(void *context, struct fs_file *file) {
    static_cast<void>(context);
    if (file->data) {
        delete file->data;
    }
}

static valiant::httpd::HTTPDHandlers camera_http_handlers = {
    .fs_open_custom = camera_http_fs_open_custom,
    .fs_close_custom = camera_http_fs_close_custom,
};

namespace valiant {
namespace oobe {

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void* param) {
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

void CameraTask(void *param) {
    vTaskSuspend(NULL);

    while (true) {
        auto input = std::make_unique<uint8_t[]>(kPosenetSize);
        valiant::camera::FrameFormat fmt;
        fmt.width = kPosenetWidth;
        fmt.height = kPosenetHeight;
        fmt.fmt = valiant::camera::Format::RGB;
        fmt.preserve_ratio = false;
        fmt.buffer = input.get();
        valiant::CameraTask::GetFrame({fmt});

        {
            valiant::MutexLock lock(posenet_input_mtx);
            if (!posenet_input) {
                posenet_input = std::make_unique<uint8_t[]>(kPosenetSize);
                memcpy(posenet_input.get(), input.get(), kPosenetSize);
            }
        }
        {
            valiant::MutexLock lock(camera_output_mtx);
            camera_output.reset(input.release());
        }

        if (pause_processing) {
            printf("suspend camera\r\n");
            vTaskSuspend(NULL);
            printf("resumed camera\r\n");
        }
        taskYIELD();
    }

    vTaskSuspend(NULL);
}

void PosenetTask(void *param) {
    vTaskSuspend(NULL);

    valiant::posenet::Output output;
    while (true) {
        TfLiteTensor *input = valiant::posenet::input();
        {
            valiant::MutexLock lock(posenet_input_mtx);
            if (!posenet_input) {
                taskYIELD();
                continue;
            }
            memcpy(tflite::GetTensorData<uint8_t>(input), posenet_input.get(), kPosenetSize);
            posenet_input.reset(nullptr);
        }

        valiant::posenet::loop(&output, false);
        int good_poses_count = 0;
        for (int i = 0; i < valiant::posenet::kPoses; ++i) {
            if (output.poses[i].score > kThreshold) {
                good_poses_count++;
            }
        }
        if (good_poses_count) {
            valiant::MutexLock lock(posenet_output_mtx);
            posenet_output.reset(nullptr);
            posenet_output = std::make_unique<valiant::posenet::Output>(output);
            posenet_output_time = xTaskGetTickCount();
        }

        if (pause_processing) {
            printf("suspend posenet\r\n");
            vTaskSuspend(NULL);
            printf("resumed posenet\r\n");
        }
        taskYIELD();
    }

    vTaskSuspend(NULL);
}

void main() {
    TaskHandle_t camera_task, posenet_task;
    xTaskCreate(CameraTask, "oobe_camera_task", configMINIMAL_STACK_SIZE * 30, nullptr, APP_TASK_PRIORITY, &camera_task);
    xTaskCreate(PosenetTask, "oobe_posenet_task", configMINIMAL_STACK_SIZE * 30, nullptr, APP_TASK_PRIORITY, &posenet_task);
    posenet_input_mtx = xSemaphoreCreateMutex();
    camera_output_mtx = xSemaphoreCreateMutex();
    posenet_output_mtx = xSemaphoreCreateMutex();

    valiant::httpd::Init();
    valiant::httpd::RegisterHandlerForPath("/", &camera_http_handlers);

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
        pause_processing = false;
        printf("CM7 awoken\r\n");
        valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
        vTaskResume(camera_task);
        vTaskResume(posenet_task);
        posenet_output_time = xTaskGetTickCount();

        while (true) {
            TickType_t now = xTaskGetTickCount();
            if ((now - posenet_output_time) > pdMS_TO_TICKS(5000)) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        printf("Transition back to M4\r\n");
        pause_processing = true;

        while (eTaskGetState(camera_task) != eSuspended && eTaskGetState(posenet_task) != eSuspended) {
            taskYIELD();
        }

        valiant::CameraTask::GetSingleton()->Disable();
        valiant::ipc::Message msg;
        msg.type = valiant::ipc::MessageType::APP;
        valiant::IPCM7::GetSingleton()->SendMessage(msg);
        vTaskSuspend(NULL);
    }
}

}  // namespace oobe
}  // namespace valiant

extern "C" void app_main(void *param) {
    valiant::oobe::main();
    vTaskSuspend(NULL);
}
