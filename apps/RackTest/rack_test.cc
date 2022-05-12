#include <array>
#include <memory>

#include "apps/RackTest/rack_test_ipc.h"
#include "libs/CoreMark/core_portme.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/utils.h"
#include "libs/posenet/posenet.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

#if defined TEST_WIFI
#include "libs/base/wifi.h"
#endif

namespace {
constexpr char kMethodM4XOR[] = "m4_xor";
constexpr char kMethodM4CoreMark[] = "m4_coremark";
constexpr char kMethodM7CoreMark[] = "m7_coremark";
constexpr char kMethodGetFrame[] = "get_frame";

std::vector<uint8_t> camera_rgb;

void HandleAppMessage(
    const uint8_t data[coral::micro::ipc::kMessageBufferDataSize],
    void* param) {
    auto rpc_task_handle = reinterpret_cast<TaskHandle_t>(param);
    const auto* app_message = reinterpret_cast<const RackTestAppMessage*>(data);
    switch (app_message->message_type) {
        case RackTestAppMessageType::XOR: {
            xTaskNotify(rpc_task_handle, app_message->message.xor_value,
                        eSetValueWithOverwrite);
            break;
        }
        case RackTestAppMessageType::COREMARK: {
            xTaskNotify(rpc_task_handle, 0, eSetValueWithOverwrite);
            break;
        }
        default:
            printf("Unknown message type\r\n");
    }
}

void PosenetStressRun(struct jsonrpc_request* request) {
    coral::micro::posenet::Output output{};
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::STREAMING);
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(
        coral::micro::PerformanceMode::kMax);
    bool loopSuccess;
    int iterations;

    if (!coral::micro::posenet::setup()) {
        printf("setup() failed\r\n");
        coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
        coral::micro::CameraTask::GetSingleton()->SetPower(false);
        jsonrpc_return_error(request, -1, "Posenet setup() failed", nullptr);
        return;
    }
    coral::micro::posenet::loop(&output);
    printf("Posenet static datatest finished.\r\n");

    coral::micro::CameraTask::GetSingleton()->DiscardFrames(100);
    if (!coral::micro::testlib::JsonRpcGetIntegerParam(request, "iterations",
                                                       &iterations)) {
        coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
        coral::micro::CameraTask::GetSingleton()->SetPower(false);
        return;
    }

    for (int i = 0; i < iterations; i++) {
        TfLiteTensor* input = coral::micro::posenet::input();
        coral::micro::camera::FrameFormat fmt{};
        fmt.filter = coral::micro::camera::FilterMethod::BILINEAR;
        fmt.width = input->dims->data[2];
        fmt.height = input->dims->data[1];
        fmt.fmt = coral::micro::camera::Format::RGB;
        fmt.preserve_ratio = false;
        fmt.buffer = tflite::GetTensorData<uint8_t>(input);
        coral::micro::CameraTask::GetFrame({fmt});
        loopSuccess = coral::micro::posenet::loop(&output);
        if (!loopSuccess) {
            coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
            coral::micro::CameraTask::GetSingleton()->SetPower(false);
            jsonrpc_return_error(request, -1, "Posenet loop() returned failure",
                                 nullptr);
            return;
        }
    }
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    jsonrpc_return_success(request, "{}");
}

void M4XOR(struct jsonrpc_request* request) {
    std::string value_string;
    if (!coral::micro::testlib::JsonRpcGetStringParam(request, "value",
                                                      &value_string))
        return;

    if (!coral::micro::IPCM7::GetSingleton()->M4IsAlive(1000 /*ms*/)) {
        jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
        return;
    }

    auto value =
        reinterpret_cast<uint32_t>(strtoul(value_string.c_str(), nullptr, 10));
    coral::micro::ipc::Message msg{};
    msg.type = coral::micro::ipc::MessageType::APP;
    auto* app_message =
        reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
    app_message->message_type = RackTestAppMessageType::XOR;
    app_message->message.xor_value = value;
    coral::micro::IPCM7::GetSingleton()->SendMessage(msg);

    // hang out here and wait for an event.
    uint32_t xor_value;
    if (xTaskNotifyWait(0, 0, &xor_value, pdMS_TO_TICKS(1000)) != pdTRUE) {
        jsonrpc_return_error(request, -1,
                             "Timed out waiting for response from M4", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{%Q:%lu}", "value", xor_value);
}

void M4CoreMark(struct jsonrpc_request* request) {
    auto* ipc = coral::micro::IPCM7::GetSingleton();
    if (!ipc->M4IsAlive(1000 /*ms*/)) {
        jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
        return;
    }

    char coremark_buffer[MAX_COREMARK_BUFFER];
    coral::micro::ipc::Message msg{};
    msg.type = coral::micro::ipc::MessageType::APP;
    auto* app_message =
        reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
    app_message->message_type = RackTestAppMessageType::COREMARK;
    app_message->message.buffer_ptr = coremark_buffer;
    coral::micro::IPCM7::GetSingleton()->SendMessage(msg);

    if (xTaskNotifyWait(0, 0, nullptr, pdMS_TO_TICKS(30000)) != pdTRUE) {
        jsonrpc_return_error(request, -1,
                             "Timed out waiting for response from M4", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results",
                           coremark_buffer);
}

void M7CoreMark(struct jsonrpc_request* request) {
    char coremark_buffer[MAX_COREMARK_BUFFER];
    RunCoreMark(coremark_buffer);
    jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results",
                           coremark_buffer);
}

void GetFrame(struct jsonrpc_request* request) {
    int rpc_width, rpc_height;
    std::string rpc_format;
    bool rpc_width_valid = coral::micro::testlib::JsonRpcGetIntegerParam(
        request, "width", &rpc_width);
    bool rpc_height_valid = coral::micro::testlib::JsonRpcGetIntegerParam(
        request, "height", &rpc_height);
    bool rpc_format_valid = coral::micro::testlib::JsonRpcGetStringParam(
        request, "format", &rpc_format);

    int width = rpc_width_valid ? rpc_width : coral::micro::CameraTask::kWidth;
    int height =
        rpc_height_valid ? rpc_height : coral::micro::CameraTask::kHeight;
    coral::micro::camera::Format format = coral::micro::camera::Format::RGB;

    if (rpc_format_valid) {
        constexpr char kFormatRGB[] = "RGB";
        constexpr char kFormatGrayscale[] = "L";
        if (memcmp(rpc_format.c_str(), kFormatRGB,
                   std::min(rpc_format.length(), strlen(kFormatRGB))) == 0) {
            format = coral::micro::camera::Format::RGB;
        }
        if (memcmp(rpc_format.c_str(), kFormatGrayscale,
                   std::min(rpc_format.length(), strlen(kFormatGrayscale))) ==
            0) {
            format = coral::micro::camera::Format::Y8;
        }
    }

    camera_rgb.resize(width * height *
                      coral::micro::CameraTask::FormatToBPP(format));

    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::camera::TestPattern pattern =
        coral::micro::camera::TestPattern::COLOR_BAR;
    coral::micro::CameraTask::GetSingleton()->SetTestPattern(pattern);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::STREAMING);
    coral::micro::camera::FrameFormat fmt_rgb{};

    fmt_rgb.fmt = format;
    fmt_rgb.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt_rgb.width = width;
    fmt_rgb.height = height;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb.data();

    bool success = coral::micro::CameraTask::GetFrame({fmt_rgb});
    coral::micro::CameraTask::GetSingleton()->SetPower(false);

    if (success)
        jsonrpc_return_success(request, "{}");
    else
        jsonrpc_return_error(request, -1, "Call to GetFrame returned false.",
                             nullptr);
}

coral::micro::HttpServer::Content UriHandler(const char* name) {
    if (std::strcmp("/camera.rgb", name) == 0)
        return coral::micro::HttpServer::Content{std::move(camera_rgb)};
    return {};
}
}  // namespace

extern "C" void app_main(void* param) {
    coral::micro::IPCM7::GetSingleton()->RegisterAppMessageHandler(
        HandleAppMessage, xTaskGetHandle(TCPIP_THREAD_NAME));
    jsonrpc_init(nullptr, nullptr);
#if defined(TEST_WIFI)
    if (!coral::micro::TurnOnWiFi()) {
        printf("Wi-Fi failed to come up (is the Wi-Fi board attached?)\r\n");
        vTaskSuspend(nullptr);
    }
    jsonrpc_export(coral::micro::testlib::kMethodWifiSetAntenna,
                   coral::micro::testlib::WifiSetAntenna);
    jsonrpc_export(coral::micro::testlib::kMethodWifiScan,
                   coral::micro::testlib::WifiScan);
    jsonrpc_export(coral::micro::testlib::kMethodWifiConnect,
                   coral::micro::testlib::WifiConnect);
    jsonrpc_export(coral::micro::testlib::kMethodWifiDisconnect,
                   coral::micro::testlib::WifiDisconnect);
    jsonrpc_export(coral::micro::testlib::kMethodWifiGetIp,
                   coral::micro::testlib::WifiGetIp);
    jsonrpc_export(coral::micro::testlib::kMethodWifiGetStatus,
                   coral::micro::testlib::WifiGetStatus);
#endif
    jsonrpc_export(coral::micro::testlib::kMethodGetSerialNumber,
                   coral::micro::testlib::GetSerialNumber);
    jsonrpc_export(coral::micro::testlib::kMethodRunTestConv1,
                   coral::micro::testlib::RunTestConv1);
    jsonrpc_export(coral::micro::testlib::kMethodSetTPUPowerState,
                   coral::micro::testlib::SetTPUPowerState);
    jsonrpc_export(coral::micro::testlib::kMethodPosenetStressRun,
                   PosenetStressRun);
    jsonrpc_export(coral::micro::testlib::kMethodBeginUploadResource,
                   coral::micro::testlib::BeginUploadResource);
    jsonrpc_export(coral::micro::testlib::kMethodUploadResourceChunk,
                   coral::micro::testlib::UploadResourceChunk);
    jsonrpc_export(coral::micro::testlib::kMethodDeleteResource,
                   coral::micro::testlib::DeleteResource);
    jsonrpc_export(coral::micro::testlib::kMethodRunClassificationModel,
                   coral::micro::testlib::RunClassificationModel);
    jsonrpc_export(coral::micro::testlib::kMethodRunDetectionModel,
                   coral::micro::testlib::RunDetectionModel);
    jsonrpc_export(coral::micro::testlib::kMethodStartM4,
                   coral::micro::testlib::StartM4);
    jsonrpc_export(coral::micro::testlib::kMethodGetTemperature,
                   coral::micro::testlib::GetTemperature);
    jsonrpc_export(kMethodM4XOR, M4XOR);
    jsonrpc_export(coral::micro::testlib::kMethodCaptureTestPattern,
                   coral::micro::testlib::CaptureTestPattern);
    jsonrpc_export(kMethodM4CoreMark, M4CoreMark);
    jsonrpc_export(kMethodM7CoreMark, M7CoreMark);
    jsonrpc_export(kMethodGetFrame, GetFrame);
    jsonrpc_export(coral::micro::testlib::kMethodCaptureAudio,
                   coral::micro::testlib::CaptureAudio);
    coral::micro::JsonRpcHttpServer server;
    server.AddUriHandler(UriHandler);
    coral::micro::UseHttpServer(&server);
    vTaskSuspend(nullptr);
}
