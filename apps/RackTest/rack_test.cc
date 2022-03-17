#include "apps/RackTest/rack_test_ipc.h"
#include "libs/base/httpd.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/utils.h"
#include "libs/CoreMark/core_portme.h"
#include "libs/posenet/posenet.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

#include <array>
#include <memory>

static constexpr char kMethodM4XOR[] = "m4_xor";
static constexpr char kMethodM4CoreMark[] = "m4_coremark";
static constexpr char kMethodM7CoreMark[] = "m7_coremark";
static constexpr char kMethodGetFrame[] = "get_frame";
static std::unique_ptr<uint8_t[]> camera_rgb;

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void *param) {
    auto rpc_task_handle = reinterpret_cast<TaskHandle_t>(param);
    const auto* app_message = reinterpret_cast<const RackTestAppMessage*>(data);
    switch (app_message->message_type) {
        case RackTestAppMessageType::XOR: {
            xTaskNotify(rpc_task_handle, app_message->message.xor_value, eSetValueWithOverwrite);
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

void PosenetStressRun(struct jsonrpc_request *request) {
    valiant::posenet::Output output{};
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    bool loopSuccess;
    int iterations;

    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
        valiant::CameraTask::GetSingleton()->SetPower(false);
        jsonrpc_return_error(request, -1, "Posenet setup() failed", nullptr);
        return;
    }
    valiant::posenet::loop(&output);
    printf("Posenet static datatest finished.\r\n");

    valiant::CameraTask::GetSingleton()->DiscardFrames(100);
    if (!valiant::testlib::JsonRpcGetIntegerParam(request, "iterations", &iterations)) {
         valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
         valiant::CameraTask::GetSingleton()->SetPower(false);
         return;
    }

    for (int i = 0; i < iterations; i++) {
        TfLiteTensor* input = valiant::posenet::input();
        valiant::camera::FrameFormat fmt{};
        fmt.width = input->dims->data[2];
        fmt.height = input->dims->data[1];
        fmt.fmt = valiant::camera::Format::RGB;
        fmt.preserve_ratio = false;
        fmt.buffer = tflite::GetTensorData<uint8_t>(input);
        valiant::CameraTask::GetFrame({fmt});
        loopSuccess = valiant::posenet::loop(&output);
        if (!loopSuccess) {
            valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
            valiant::CameraTask::GetSingleton()->SetPower(false);
            jsonrpc_return_error(request, -1, "Posenet loop() returned failure", nullptr);
            return;
        }

    }
    valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
    valiant::CameraTask::GetSingleton()->SetPower(false);
    jsonrpc_return_success(request, "{}");
}

static void M4XOR(struct jsonrpc_request *request) {
    std::vector<char> value_string;
    if (!valiant::testlib::JsonRpcGetStringParam(request, "value", &value_string))
        return;

    if (!valiant::IPCM7::GetSingleton()->M4IsAlive(1000/*ms*/)) {
        jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
        return;
    }

    auto value = reinterpret_cast<uint32_t>(strtoul(value_string.data(), nullptr, 10));
    valiant::ipc::Message msg{};
    msg.type = valiant::ipc::MessageType::APP;
    auto* app_message = reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
    app_message->message_type = RackTestAppMessageType::XOR;
    app_message->message.xor_value = value;
    valiant::IPCM7::GetSingleton()->SendMessage(msg);

    // hang out here and wait for an event.
    uint32_t xor_value;
    if (xTaskNotifyWait(0, 0, &xor_value, pdMS_TO_TICKS(1000)) != pdTRUE) {
        jsonrpc_return_error(request, -1, "Timed out waiting for response from M4", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{%Q:%lu}", "value", xor_value);
}

static void M4CoreMark(struct jsonrpc_request *request) {
    auto* ipc = valiant::IPCM7::GetSingleton();
    if (!ipc->M4IsAlive(1000/*ms*/)) {
        jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
        return;
    }

    char coremark_buffer[MAX_COREMARK_BUFFER];
    valiant::ipc::Message msg{};
    msg.type = valiant::ipc::MessageType::APP;
    auto* app_message = reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
    app_message->message_type = RackTestAppMessageType::COREMARK;
    app_message->message.buffer_ptr = coremark_buffer;
    valiant::IPCM7::GetSingleton()->SendMessage(msg);

    if (xTaskNotifyWait(0, 0, nullptr, pdMS_TO_TICKS(30000)) != pdTRUE) {
        jsonrpc_return_error(request, -1, "Timed out waiting for response from M4", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results", coremark_buffer);
}

static void M7CoreMark(struct jsonrpc_request *request) {
    char coremark_buffer[MAX_COREMARK_BUFFER];
    RunCoreMark(coremark_buffer);
    jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results", coremark_buffer);
}

static void GetFrame(struct jsonrpc_request *request) {
    if(!camera_rgb){
        camera_rgb = std::make_unique<uint8_t[]>(valiant::CameraTask::kWidth * valiant::CameraTask::kHeight *
            valiant::CameraTask::FormatToBPP(valiant::camera::Format::RGB));
    }
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::camera::TestPattern pattern = valiant::camera::TestPattern::COLOR_BAR;
    valiant::CameraTask::GetSingleton()->SetTestPattern(pattern);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::camera::FrameFormat fmt_rgb{};

    fmt_rgb.fmt = valiant::camera::Format::RGB;
    fmt_rgb.width = valiant::CameraTask::kWidth;
    fmt_rgb.height = valiant::CameraTask::kHeight;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb.get();


    bool success = (valiant::CameraTask::GetSingleton()->GetFrame({fmt_rgb}));

    if (success)
        jsonrpc_return_success(request, "{}");
    else
        jsonrpc_return_error(request, -1, "Call to GetFrame returned false.", nullptr);

    valiant::CameraTask::GetSingleton()->SetPower(false);
}

static int fs_open_custom(void *context, struct fs_file *file, const char *name) {
    if (strncmp(name, "/colorbar.raw", strlen(name)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = reinterpret_cast<const char*>(camera_rgb.release());
        file->len = static_cast<int>(valiant::CameraTask::kWidth * valiant::CameraTask::kHeight *
                                     valiant::CameraTask::FormatToBPP(valiant::camera::Format::RGB));
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    return 0;
}

static void fs_close_custom(void* context, struct fs_file *file) {
    delete file->data;
}

extern "C" void app_main(void *param) {
    valiant::IPCM7::GetSingleton()->RegisterAppMessageHandler(HandleAppMessage, xTaskGetHandle(TCPIP_THREAD_NAME));
    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    if (!rpc_server_io_http.Init()) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(nullptr);
    }

    valiant::httpd::HTTPDHandlers handlers{};
    handlers.fs_open_custom = fs_open_custom;
    handlers.fs_close_custom = fs_close_custom;

    valiant::httpd::Init();
    valiant::httpd::RegisterHandlerForPath("/", &handlers);

    jsonrpc_init(nullptr, nullptr);
    jsonrpc_export(valiant::testlib::kMethodGetSerialNumber,
                   valiant::testlib::GetSerialNumber);
    jsonrpc_export(valiant::testlib::kMethodRunTestConv1,
                   valiant::testlib::RunTestConv1);
    jsonrpc_export(valiant::testlib::kMethodSetTPUPowerState,
                   valiant::testlib::SetTPUPowerState);
    jsonrpc_export(valiant::testlib::kMethodPosenetStressRun, PosenetStressRun);
    jsonrpc_export(valiant::testlib::kMethodBeginUploadResource,
                   valiant::testlib::BeginUploadResource);
    jsonrpc_export(valiant::testlib::kMethodUploadResourceChunk,
                   valiant::testlib::UploadResourceChunk);
    jsonrpc_export(valiant::testlib::kMethodDeleteResource,
                   valiant::testlib::DeleteResource);
    jsonrpc_export(valiant::testlib::kMethodRunClassificationModel,
                   valiant::testlib::RunClassificationModel);
    jsonrpc_export(valiant::testlib::kMethodRunDetectionModel,
                   valiant::testlib::RunDetectionModel);
    jsonrpc_export(valiant::testlib::kMethodStartM4,
                   valiant::testlib::StartM4);
    jsonrpc_export(valiant::testlib::kMethodGetTemperature,
                   valiant::testlib::GetTemperature);
    jsonrpc_export(kMethodM4XOR, M4XOR);
    jsonrpc_export(valiant::testlib::kMethodCaptureTestPattern,
                   valiant::testlib::CaptureTestPattern);
    jsonrpc_export(kMethodM4CoreMark, M4CoreMark);
    jsonrpc_export(kMethodM7CoreMark, M7CoreMark);
    jsonrpc_export(kMethodGetFrame, GetFrame);
    jsonrpc_export(valiant::testlib::kMethodCaptureAudio,
                   valiant::testlib::CaptureAudio);
    rpc_server_io_http.SetContext(&jsonrpc_default_context);
    vTaskSuspend(nullptr);
}
