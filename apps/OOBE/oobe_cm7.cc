#include "apps/OOBE/oobe_json.h"

#include "libs/base/httpd.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/base/utils.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
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
extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}
#endif  // defined(OOBE_DEMO_WIFI)


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
std::vector<uint8_t> posenet_input;

static SemaphoreHandle_t camera_output_mtx;
std::vector<uint8_t> camera_output;

static SemaphoreHandle_t posenet_output_mtx;
std::unique_ptr<valiant::posenet::Output> posenet_output;
TickType_t posenet_output_time;

constexpr unsigned int kCameraPextension = 0xdeadbeefU;
constexpr unsigned int kPosePextension = 0xbadbeeefU;

static bool DynamicFileHandler(const char *name, std::vector<uint8_t>* buffer) {
    if (std::strcmp(name, "/camera") == 0) {
        valiant::MutexLock lock(camera_output_mtx);
        *buffer = std::move(camera_output);
        return true;
    }

    if (std::strcmp(name, "/pose") == 0) {
        valiant::MutexLock lock(posenet_output_mtx);
        if (!posenet_output) return false;
        *buffer = valiant::oobe::CreatePoseJSON(*posenet_output, kThreshold);
        posenet_output.reset();
        return true;
    }

    return false;
}

namespace valiant {
namespace oobe {

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void* param) {
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

void CameraTask(void *param) {
    vTaskSuspend(NULL);

    while (true) {
        std::vector<uint8_t> input(kPosenetSize);
        valiant::camera::FrameFormat fmt;
        fmt.width = kPosenetWidth;
        fmt.height = kPosenetHeight;
        fmt.fmt = valiant::camera::Format::RGB;
        fmt.preserve_ratio = false;
        fmt.buffer = input.data();
        valiant::CameraTask::GetFrame({fmt});

        {
            valiant::MutexLock lock(posenet_input_mtx);
            if (posenet_input.empty()) {
                posenet_input = input;
            }
        }
        {
            valiant::MutexLock lock(camera_output_mtx);
            camera_output = std::move(input);
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
            if (posenet_input.empty()) {
                taskYIELD();
                continue;
            }
            memcpy(tflite::GetTensorData<uint8_t>(input), posenet_input.data(), kPosenetSize);
            posenet_input.resize(0);
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

#if defined(OOBE_DEMO_WIFI)
static bool ConnectToWifi() {
    std::string wifi_ssid, wifi_psk;
    bool have_ssid = valiant::utils::GetWifiSSID(&wifi_ssid);
    bool have_psk = valiant::utils::GetWifiPSK(&wifi_psk);

    if (have_ssid) {
        WIFI_On();
        WIFIReturnCode_t xWifiStatus;
        WIFINetworkParams_t xNetworkParams;
        xNetworkParams.pcSSID = wifi_ssid.c_str();
        xNetworkParams.ucSSIDLength = wifi_ssid.length();
        if (have_psk) {
            xNetworkParams.pcPassword = wifi_psk.c_str();
            xNetworkParams.ucPasswordLength = wifi_psk.length();
            xNetworkParams.xSecurity = eWiFiSecurityWPA2;
        } else {
            xNetworkParams.pcPassword = "";
            xNetworkParams.ucPasswordLength = 0;
            xNetworkParams.xSecurity = eWiFiSecurityOpen;
        }
        xWifiStatus = WIFI_ConnectAP(&xNetworkParams);

        if (xWifiStatus != eWiFiSuccess) {
            printf("failed to connect to %s\r\n", wifi_ssid.c_str());
            return false;
        }
    } else {
        printf("No Wi-Fi SSID provided\r\n");
        return false;
    }
    return true;
}
#endif // defined(OOBE_DEMO_WIFI)

void main() {
    TaskHandle_t camera_task, posenet_task;
    xTaskCreate(CameraTask, "oobe_camera_task", configMINIMAL_STACK_SIZE * 30, nullptr, APP_TASK_PRIORITY, &camera_task);
    xTaskCreate(PosenetTask, "oobe_posenet_task", configMINIMAL_STACK_SIZE * 30, nullptr, APP_TASK_PRIORITY, &posenet_task);
    posenet_input_mtx = xSemaphoreCreateMutex();
    camera_output_mtx = xSemaphoreCreateMutex();
    posenet_output_mtx = xSemaphoreCreateMutex();

// For the OOBE Demo, bring up WiFi and Ethernet. For now these are active
// but unused.
#if defined(OOBE_DEMO_ETHERNET)
    valiant::InitializeEthernet(false);
#elif defined(OOBE_DEMO_WIFI)
    ConnectToWifi();
    if (!ConnectToWifi()) {
        // If connecting to wi-fi fails, turn our LEDs on solid, and halt.
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kPowerLED, true);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, true);
        valiant::gpio::SetGpio(valiant::gpio::Gpio::kTpuLED, true);
        vTaskSuspend(NULL);
    }
#endif // defined(OOBE_DEMO_ETHERNET)

    valiant::httpd::HttpServer http_server;
    http_server.SetDynamicFileHandler(DynamicFileHandler);
    valiant::httpd::Init(&http_server);

    valiant::IPCM7::GetSingleton()->RegisterAppMessageHandler(HandleAppMessage, xTaskGetCurrentTaskHandle());
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }

    valiant::IPCM7::GetSingleton()->StartM4();

#if defined(OOBE_DEMO)
    int count = 0;
#endif  // defined (OOBE_DEMO)

    vTaskSuspend(NULL);
    while (true) {
        pause_processing = false;
        printf("CM7 awoken\r\n");
        valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
        vTaskResume(camera_task);
        vTaskResume(posenet_task);
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
#else
            TickType_t now = xTaskGetTickCount();
            if ((now - posenet_output_time) > pdMS_TO_TICKS(5000)) {
                break;
            }
#endif // defined(OOBE_DEMO)

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
