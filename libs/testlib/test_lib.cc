#include "libs/testlib/test_lib.h"

#include <array>
#include <map>

#include "libs/audio/audio_driver.h"
#include "libs/base/filesystem.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/strings.h"
#include "libs/base/tempsense.h"
#include "libs/base/timer.h"
#include "libs/base/utils.h"
#include "libs/base/wifi.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/testconv1/testconv1.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/schema/schema_generated.h"
extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

namespace coral::micro::testlib {
namespace {
AudioDriverBuffers</*NumDmaBuffers=*/4, /*DmaBufferSize=*/6 * 1024>
    g_audio_buffers;
AudioDriver g_audio_driver(g_audio_buffers);

constexpr int kTensorArenaSize = 8 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

// Map for containing uploaded resources.
// Key is the output of StrHash with the resource name as the parameter.
std::map<std::string, std::vector<uint8_t>> g_uploaded_resources;

std::unique_ptr<char[]> JSONRPCCreateParamFormatString(const char* param_name) {
    const char* param_format = "$[0].%s";
    // +1 for null terminator.
    auto size = snprintf(nullptr, 0, param_format, param_name) + 1;
    auto param_pattern = std::make_unique<char[]>(size);
    snprintf(param_pattern.get(), size, param_format, param_name);
    return param_pattern;
}

std::vector<uint8_t>* GetResource(const std::string& resource_name) {
    auto it = g_uploaded_resources.find(resource_name);
    if (it == g_uploaded_resources.end()) return nullptr;
    return &it->second;
}

namespace pended_functions {
// These functions' signatures here are required to match with the
// PendedFunction_t as described here:
// https://www.freertos.org/xTimerPendFunctionCall.html

auto WiFiSafeDisconnect = [](void* = nullptr, uint32_t = 0) {
    if (!coral::micro::DisconnectWiFi()) {
        printf("Unable to disconnect from previous wifi connection\r\n");
    }
};

auto WiFiSafeConnect = [](void* wifi_network_params, uint32_t retries) {
    WiFiSafeDisconnect();
    auto params = reinterpret_cast<WIFINetworkParams_t*>(wifi_network_params);
    coral::micro::ConnectWiFi(params, static_cast<int>(retries));
    free((void*)params->pcSSID);
    free((void*)params->pcPassword);
    free(wifi_network_params);
};
}  // namespace pended_functions
}  // namespace

void JsonRpcReturnBadParam(struct jsonrpc_request* request, const char* message,
                           const char* param_name) {
    jsonrpc_return_error(request, JSONRPC_ERROR_BAD_PARAMS, message, "{%Q:%Q}",
                         "param", param_name);
}

bool JsonRpcGetIntegerParam(struct jsonrpc_request* request,
                            const char* param_name, int* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    double value;
    if (mjson_get_number(request->params, request->params_len,
                         param_pattern.get(), &value) == 0) {
        JsonRpcReturnBadParam(request, "invalid param", param_name);
        return false;
    }

    *out = static_cast<int>(value);
    return true;
}

bool JsonRpcGetBooleanParam(struct jsonrpc_request* request,
                            const char* param_name, bool* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    int value;
    if (mjson_get_bool(request->params, request->params_len,
                       param_pattern.get(), &value) == 0) {
        JsonRpcReturnBadParam(request, "invalid param", param_name);
        return false;
    }

    *out = static_cast<bool>(value);
    return true;
}

bool JsonRpcGetStringParam(struct jsonrpc_request* request,
                           const char* param_name, std::string* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    ssize_t size = 0;
    int tok = mjson_find(request->params, request->params_len,
                         param_pattern.get(), nullptr, &size);
    if (tok != MJSON_TOK_STRING) {
        JsonRpcReturnBadParam(request, "invalid param", param_name);
        return false;
    }

    out->resize(size);
    auto len = mjson_get_string(request->params, request->params_len,
                                param_pattern.get(), out->data(), out->size());
    out->resize(len);
    return true;
}

bool JsonRpcGetBase64Param(struct jsonrpc_request* request,
                           const char* param_name, std::vector<uint8_t>* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    ssize_t size = 0;
    int tok = mjson_find(request->params, request->params_len,
                         param_pattern.get(), nullptr, &size);
    if (tok != MJSON_TOK_STRING) {
        JsonRpcReturnBadParam(request, "invalid param", param_name);
        return false;
    }

    // `size` includes both quotes, `size - 2` is the real string size. Base64
    // encodes every 3 bytes as 4 chars. Buffer size of `3 * ceil(size - 2) / 4`
    // should be enough.
    out->resize(3 * (((size - 2) + 3) / 4));
    auto len = mjson_get_base64(request->params, request->params_len,
                                param_pattern.get(),
                                reinterpret_cast<char*>(out->data()), size);
    out->resize(len);
    return true;
}

// Implementation of "get_serial_number" RPC.
// Returns JSON results with the key "serial_number" and the serial, as a
// string.
void GetSerialNumber(struct jsonrpc_request* request) {
    std::string serial = coral::micro::utils::GetSerialNumber();
    jsonrpc_return_success(request, "{%Q:%.*Q}", "serial_number", serial.size(),
                           serial.c_str());
}

// Implements the "run_testconv1" RPC.
// Runs the simple "testconv1" model using the TPU.
// NOTE: The TPU power must be enabled for this RPC to succeed.
void RunTestConv1(struct jsonrpc_request* request) {
    if (!coral::micro::EdgeTpuTask::GetSingleton()->GetPower()) {
        jsonrpc_return_error(request, -1, "TPU power is not enabled", nullptr);
        return;
    }
    coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!coral::micro::testconv1::setup()) {
        jsonrpc_return_error(request, -1, "testconv1 setup failed", nullptr);
        return;
    }
    if (!coral::micro::testconv1::loop()) {
        jsonrpc_return_error(request, -1, "testconv1 loop failed", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{}");
}

// Implements the "set_tpu_power_state" RPC.
// Takes one parameter, "enable" -- a boolean indicating the state to set.
// Returns success or failure.
void SetTPUPowerState(struct jsonrpc_request* request) {
    bool enable;
    if (!JsonRpcGetBooleanParam(request, "enable", &enable)) return;

    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(enable);
    jsonrpc_return_success(request, "{}");
}

void BeginUploadResource(struct jsonrpc_request* request) {
    std::string resource_name;
    if (!JsonRpcGetStringParam(request, "name", &resource_name)) return;

    int resource_size;
    if (!JsonRpcGetIntegerParam(request, "size", &resource_size)) return;

    g_uploaded_resources[resource_name].resize(resource_size);
    jsonrpc_return_success(request, "{}");
}

void UploadResourceChunk(struct jsonrpc_request* request) {
    std::string resource_name;
    if (!JsonRpcGetStringParam(request, "name", &resource_name)) return;

    auto* resource = GetResource(resource_name);
    if (!resource) {
        jsonrpc_return_error(request, -1, "unknown resource", nullptr);
        return;
    }

    int offset;
    if (!JsonRpcGetIntegerParam(request, "offset", &offset)) return;

    std::vector<uint8_t> data;
    if (!JsonRpcGetBase64Param(request, "data", &data)) return;
    std::memcpy(resource->data() + offset, data.data(), data.size());

    jsonrpc_return_success(request, "{}");
}

void DeleteResource(struct jsonrpc_request* request) {
    std::string resource_name;
    if (!JsonRpcGetStringParam(request, "name", &resource_name)) return;

    auto it = g_uploaded_resources.find(resource_name);
    if (it == g_uploaded_resources.end()) {
        jsonrpc_return_error(request, -1, "unknown resource", nullptr);
        return;
    }

    g_uploaded_resources.erase(it);
    jsonrpc_return_success(request, "{}");
}

void FetchResource(struct jsonrpc_request* request) {
    std::string resource_name;
    if (!JsonRpcGetStringParam(request, "name", &resource_name)) {
        jsonrpc_return_error(request, -1, "missing resource name", nullptr);
        return;
    }

    auto* resource = GetResource(resource_name);
    if (!resource) {
        jsonrpc_return_error(request, -1, "Unknown resource", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q:%V}", "data", resource->size(),
                           resource->data());
}

void RunDetectionModel(struct jsonrpc_request* request) {
    std::string model_resource_name, image_resource_name;
    int image_width, image_height, image_depth;

    if (!JsonRpcGetStringParam(request, "model_resource_name",
                               &model_resource_name))
        return;

    if (!JsonRpcGetStringParam(request, "image_resource_name",
                               &image_resource_name))
        return;

    if (!JsonRpcGetIntegerParam(request, "image_width", &image_width)) return;

    if (!JsonRpcGetIntegerParam(request, "image_height", &image_height)) return;

    if (!JsonRpcGetIntegerParam(request, "image_depth", &image_depth)) return;

    const auto* model_resource = GetResource(model_resource_name);
    if (!model_resource) {
        jsonrpc_return_error(request, -1, "missing model resource", nullptr);
        return;
    }
    const auto* image_resource = GetResource(image_resource_name);
    if (!image_resource) {
        jsonrpc_return_error(request, -1, "missing image resource", nullptr);
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_resource->data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        jsonrpc_return_error(request, -1, "model schema version unsupported",
                             nullptr);
        return;
    }

    tflite::MicroErrorReporter error_reporter;
    std::shared_ptr<EdgeTpuContext> context =
        EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        jsonrpc_return_error(request, -1, "failed to open TPU", nullptr);
        return;
    }

    tflite::MicroMutableOpResolver<3> resolver;
    resolver.AddDequantize();
    resolver.AddDetectionPostprocess();
    resolver.AddCustom(kCustomOp, RegisterCustomOp());

    tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                         kTensorArenaSize, &error_reporter);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to allocate tensors",
                             nullptr);
        return;
    }

    auto* input_tensor = interpreter.input_tensor(0);
    auto* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    tensorflow::ImageDims tensor_dims = {input_tensor->dims->data[1],
                                         input_tensor->dims->data[2],
                                         input_tensor->dims->data[3]};
    auto preprocess_start = coral::micro::timer::micros();
    if (!tensorflow::ResizeImage({image_height, image_width, image_depth},
                                 image_resource->data(), tensor_dims,
                                 input_tensor_data)) {
        jsonrpc_return_error(request, -1, "Failed to resize input image",
                             nullptr);
        return;
    }
    auto preprocess_latency = coral::micro::timer::micros() - preprocess_start;

    // The first Invoke is slow due to model transfer. Run an Invoke
    // but ignore the results.
    if (interpreter.Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter",
                             nullptr);
        return;
    }

    auto invoke_start = coral::micro::timer::micros();
    if (interpreter.Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter",
                             nullptr);
        return;
    }
    auto invoke_latency = coral::micro::timer::micros() - invoke_start;

    // Return results and check on host side
    auto results = tensorflow::GetDetectionResults(&interpreter, 0.7, 3);
    if (results.empty()) {
        jsonrpc_return_error(request, -1, "no results above threshold",
                             nullptr);
        return;
    }
    const auto& top_result = results.at(0);
    jsonrpc_return_success(
        request, "{%Q: %d, %Q: %g, %Q: %g, %Q: %g, %Q: %g, %Q: %g, %Q:%d}",
        "id", top_result.id, "score", top_result.score, "xmin",
        top_result.bbox.xmin, "xmax", top_result.bbox.xmax, "ymin",
        top_result.bbox.ymin, "ymax", top_result.bbox.ymax, "latency",
        (preprocess_latency + invoke_latency));
}

void RunClassificationModel(struct jsonrpc_request* request) {
    std::string model_resource_name, image_resource_name;
    int image_width, image_height, image_depth;

    if (!JsonRpcGetStringParam(request, "model_resource_name",
                               &model_resource_name))
        return;

    if (!JsonRpcGetStringParam(request, "image_resource_name",
                               &image_resource_name))
        return;

    if (!JsonRpcGetIntegerParam(request, "image_width", &image_width)) return;

    if (!JsonRpcGetIntegerParam(request, "image_height", &image_height)) return;

    if (!JsonRpcGetIntegerParam(request, "image_depth", &image_depth)) return;

    const auto* model_resource = GetResource(model_resource_name);
    if (!model_resource) {
        jsonrpc_return_error(request, -1, "missing model resource", nullptr);
        return;
    }
    const auto* image_resource = GetResource(image_resource_name);
    if (!image_resource) {
        jsonrpc_return_error(request, -1, "missing image resource", nullptr);
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_resource->data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        jsonrpc_return_error(request, -1, "model schema version unsupported",
                             nullptr);
        return;
    }

    tflite::MicroErrorReporter error_reporter;
    std::shared_ptr<EdgeTpuContext> context =
        EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        jsonrpc_return_error(request, -1, "failed to open TPU", nullptr);
        return;
    }

    tflite::MicroMutableOpResolver<1> resolver;
    resolver.AddCustom(kCustomOp, RegisterCustomOp());
    tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                         kTensorArenaSize, &error_reporter);
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to allocate tensors",
                             nullptr);
        return;
    }

    auto* input_tensor = interpreter.input_tensor(0);
    bool needs_preprocessing =
        tensorflow::ClassificationInputNeedsPreprocessing(*input_tensor);
    uint32_t preprocess_latency = 0;
    if (needs_preprocessing) {
        uint32_t preprocess_start = coral::micro::timer::micros();
        if (!tensorflow::ClassificationPreprocess(input_tensor)) {
            jsonrpc_return_error(request, -1, "input preprocessing failed",
                                 nullptr);
            return;
        }
        uint32_t preprocess_end = coral::micro::timer::micros();
        preprocess_latency = preprocess_end - preprocess_start;
    }

    // Resize into input tensor
    tensorflow::ImageDims input_tensor_dims = {input_tensor->dims->data[1],
                                               input_tensor->dims->data[2],
                                               input_tensor->dims->data[3]};
    if (!tensorflow::ResizeImage(
            {image_height, image_width, image_depth}, image_resource->data(),
            input_tensor_dims, tflite::GetTensorData<uint8_t>(input_tensor))) {
        jsonrpc_return_error(request, -1, "failed to resize input", nullptr);
        return;
    }

    // The first Invoke is slow due to model transfer. Run an Invoke
    // but ignore the results.
    if (interpreter.Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter",
                             nullptr);
        return;
    }

    uint32_t start = coral::micro::timer::micros();
    if (interpreter.Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter",
                             nullptr);
        return;
    }
    uint32_t end = coral::micro::timer::micros();
    uint32_t latency = end - start;

    // Return results and check on host side
    auto results = tensorflow::GetClassificationResults(&interpreter, 0.0f, 1);
    if (results.empty()) {
        jsonrpc_return_error(request, -1, "no results above threshold",
                             nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q:%d, %Q:%g, %Q:%d}", "id",
                           results[0].id, "score", results[0].score, "latency",
                           latency + preprocess_latency);
}

void StartM4(struct jsonrpc_request* request) {
    auto* ipc = IPCM7::GetSingleton();
    if (!ipc->HasM4Application()) {
        jsonrpc_return_error(request, -1, "No M4 application present", nullptr);
        return;
    }

    ipc->StartM4();
    if (!ipc->M4IsAlive(1000 /* ms */)) {
        jsonrpc_return_error(request, -1, "M4 did not come to life", nullptr);
        return;
    }

    jsonrpc_return_success(request, "{}");
}

void GetTemperature(struct jsonrpc_request* request) {
    int sensor_num;
    if (!JsonRpcGetIntegerParam(request, "sensor", &sensor_num)) return;

    auto sensor = static_cast<coral::micro::tempsense::TempSensor>(sensor_num);
    if (sensor >= coral::micro::tempsense::TempSensor::kSensorCount) {
        jsonrpc_return_error(request, -1, "Invalid temperature sensor",
                             nullptr);
        return;
    }

    float temperature = coral::micro::tempsense::GetTemperature(sensor);
    jsonrpc_return_success(request, "{%Q:%g}", "temperature", temperature);
}

// Implements the "capture_test_pattern" RPC.
// Configures the sensor to test pattern mode, and captures via trigger.
// Returns success if the test pattern has the expected data, failure otherwise.
void CaptureTestPattern(struct jsonrpc_request* request) {
    if (!coral::micro::CameraTask::GetSingleton()->SetPower(true)) {
        coral::micro::CameraTask::GetSingleton()->SetPower(false);
        jsonrpc_return_error(request, -1, "unable to detect camera", nullptr);
        return;
    }
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::TRIGGER);
    coral::micro::CameraTask::GetSingleton()->SetTestPattern(
        coral::micro::camera::TestPattern::WALKING_ONES);

    bool success = true;
    // Getting this test pattern doesn't seem to always work on the first try,
    // maybe there is some undocumented pattern change time in the sensor. Allow
    // a small amount of retrying to smooth that over.
    constexpr int kRetries = 3;
    for (int i = 0; i < kRetries; ++i) {
        coral::micro::CameraTask::GetSingleton()->Trigger();
        uint8_t* buffer = nullptr;
        int index =
            coral::micro::CameraTask::GetSingleton()->GetFrame(&buffer, true);
        uint8_t expected = 0;
        success = true;
        for (unsigned int i = 0; i < coral::micro::CameraTask::kWidth *
                                         coral::micro::CameraTask::kHeight;
             ++i) {
            if (buffer[i] != expected) {
                success = false;
                break;
            }
            if (expected == 0) {
                expected = 1;
            } else {
                expected = expected << 1;
            }
        }
        coral::micro::CameraTask::GetSingleton()->ReturnFrame(index);
        if (success) {
            break;
        }
    }
    if (success) {
        jsonrpc_return_success(request, "{}", nullptr);
    } else {
        jsonrpc_return_error(request, -1, "camera test pattern mismatch",
                             nullptr);
    }
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
}

// Implements the "capture_audio" RPC.
// Attempts to capture 1 second of audio.
// Returns success, with a parameter "data" containing the captured audio in
// base64 (or failure). The audio captured is 32-bit signed PCM @ 16000Hz.
void CaptureAudio(struct jsonrpc_request* request) {
    int sample_rate_hz;
    if (!JsonRpcGetIntegerParam(request, "sample_rate_hz", &sample_rate_hz))
        return;

    auto sample_rate = CheckSampleRate(sample_rate_hz);
    if (!sample_rate.has_value()) {
        JsonRpcReturnBadParam(request, "sample rate must be 16000 or 48000 Hz",
                              "sample_rate_hz");
        return;
    }

    int duration_ms;
    if (!JsonRpcGetIntegerParam(request, "duration_ms", &duration_ms)) return;
    if (duration_ms <= 0) {
        JsonRpcReturnBadParam(request, "duration must be positive",
                              "duration_ms");
        return;
    }

    int num_buffers;
    if (!JsonRpcGetIntegerParam(request, "num_buffers", &num_buffers)) return;
    if (num_buffers < 1 ||
        num_buffers > static_cast<int>(g_audio_buffers.kNumDmaBuffers)) {
        JsonRpcReturnBadParam(request, "invalid number of DMA buffers",
                              "num_buffers");
        return;
    }

    int buffer_size_ms;
    if (!JsonRpcGetIntegerParam(request, "buffer_size_ms", &buffer_size_ms))
        return;
    if (buffer_size_ms < 1) {
        JsonRpcReturnBadParam(request, "invalid DMA buffer size",
                              "buffer_size_ms");
        return;
    }

    const AudioDriverConfig config{*sample_rate,
                                   static_cast<size_t>(num_buffers),
                                   static_cast<size_t>(buffer_size_ms)};

    if (!g_audio_buffers.CanHandle(config)) {
        jsonrpc_return_error(
            request, -1, "not enough static memory for DMA buffers", nullptr);
        return;
    }

    const int num_chunks = (duration_ms + buffer_size_ms / 2) / buffer_size_ms;

    std::vector<int32_t> samples(num_chunks * config.dma_buffer_size_samples());
    struct AudioParams {
        int32_t* first;
        int32_t* last;
    } params{samples.data(), samples.data() + samples.size()};

    g_audio_driver.Enable(
        config, &params, +[](void* param, const int32_t* buf, size_t size) {
            auto* params = static_cast<AudioParams*>(param);
            if (params->first + size <= params->last) {
                std::memcpy(params->first, buf, size * sizeof(buf[0]));
                params->first += size;
            }
        });

    // Add (chunk_duration_ms / 10) just in case. Capture is still limited by
    // the buffer size.
    vTaskDelay(
        pdMS_TO_TICKS(num_chunks * buffer_size_ms + buffer_size_ms / 10));
    g_audio_driver.Disable();

    jsonrpc_return_success(request, "{%Q: %V}", "data",
                           samples.size() * sizeof(samples[0]), samples.data());
}

void WiFiScan(struct jsonrpc_request* request) {
    auto x_scan_results = coral::micro::ScanWiFi();

    if (x_scan_results.empty()) {
        jsonrpc_return_error(request, -1, "wifi scan failed", nullptr);
        return;
    }

    std::vector<uint8_t> json;
    json.reserve(2048);

    coral::micro::StrAppend(&json, "[");
    int count{0};
    for (const auto& x_scan_result : x_scan_results) {
        if (x_scan_result.cSSID[0] != '\0') {
            coral::micro::StrAppend(&json, "\"%s\",", x_scan_result.cSSID);
            count++;
        }
    }
    if (count > 0) {
        json.pop_back();  // Remove the last comma.
    }
    coral::micro::StrAppend(&json, "]");
    jsonrpc_return_success(request, "{%Q:%s}", "SSIDs",
                           reinterpret_cast<const char*>(json.data()));
}

void WiFiConnect(struct jsonrpc_request* request) {
    std::string ssid, psk;
    if (!JsonRpcGetStringParam(request, "ssid", &ssid)) {
        JsonRpcReturnBadParam(request, "ssid must be specified", "ssid");
        return;
    }
    JsonRpcGetStringParam(request, "password",

                          &psk);  // Password is not required.;
    int retries{5};               // Default to 5.
    JsonRpcGetIntegerParam(request, "retries", &retries);

    auto* network_params = new WIFINetworkParams_t();
    network_params->pcSSID = (const char*)malloc(ssid.size() + 1);
    std::strcpy(const_cast<char*>(network_params->pcSSID), ssid.c_str());
    network_params->ucSSIDLength = ssid.size();
    network_params->pcPassword = (const char*)malloc(psk.size() + 1);
    std::strcpy(const_cast<char*>(network_params->pcPassword), psk.c_str());
    network_params->ucPasswordLength = psk.size();
    network_params->xSecurity =
        psk.empty() ? eWiFiSecurityOpen : eWiFiSecurityWPA2;

    jsonrpc_return_success(request, "{}");
    xTimerPendFunctionCall(pended_functions::WiFiSafeConnect, network_params,
                           static_cast<uint32_t>(retries), pdMS_TO_TICKS(10));
}

void WiFiDisconnect(struct jsonrpc_request* request) {
    jsonrpc_return_success(request, "{}");
    xTimerPendFunctionCall(pended_functions::WiFiSafeDisconnect, nullptr, 0,
                           pdMS_TO_TICKS(100));
}

void WiFiGetStatus(struct jsonrpc_request* request) {
    jsonrpc_return_success(request, "{%Q:%d}", "status",
                           coral::micro::WiFiIsConnected());
}

void WiFiGetIp(struct jsonrpc_request* request) {
    auto maybe_ip = coral::micro::GetWiFiIp();
    if (!maybe_ip.has_value()) {
        jsonrpc_return_error(request, -1, "Unable to get wifi ip.", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q:\"%s\"}", "ip",
                           maybe_ip.value().c_str());
}

void WiFiSetAntenna(struct jsonrpc_request* request) {
    int antenna;
    if (!JsonRpcGetIntegerParam(request, "antenna", &antenna)) return;

    if (!SetWiFiAntenna(static_cast<coral::micro::WiFiAntenna>(antenna))) {
        jsonrpc_return_error(request, -1, "invalid antenna selection", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{}");
}

}  // namespace coral::micro::testlib
