#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/tempsense.h"
#include "libs/base/timer.h"
#include "libs/base/utils.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/testlib/test_lib.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/detection.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/base64.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/schema/schema_generated.h"

#include <array>
#include <map>

// Map for containing uploaded resources.
// Key is the output of StrHash with the resource name as the parameter.
static std::map<uint64_t, std::vector<uint8_t>> uploaded_resources;

namespace valiant {
namespace testlib {

static std::unique_ptr<char[]> JSONRPCCreateParamFormatString(const char *param_name) {
    const char *param_format = "$[0].%s";
    // +1 for null terminator.
    auto size = snprintf(nullptr, 0, param_format, param_name) + 1;
    auto param_pattern = std::make_unique<char[]>(size);
    snprintf(param_pattern.get(), size, param_format, param_name);
    return param_pattern;
}

bool JSONRPCGetIntegerParam(struct jsonrpc_request* request, const char *param_name, int* out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);
    double param_double;
    int find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, param_pattern.get(), &param_double);
        *out = static_cast<int>(param_double);
    } else {
        return false;
    }
    return true;
}

bool JSONRPCGetBooleanParam(struct jsonrpc_request* request, const char *param_name, bool *out) {
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);
    int find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        int param;
        mjson_get_bool(request->params, request->params_len, param_pattern.get(), &param);
        *out = !!param;
    } else {
        return false;
    }
    return true;
}

bool JSONRPCGetStringParam(struct jsonrpc_request* request, const char *param_name, std::vector<char>* out) {
    int find_result;
    ssize_t find_size = 0;
    auto param_pattern = JSONRPCCreateParamFormatString(param_name);

    find_result = mjson_find(request->params, request->params_len, param_pattern.get(), nullptr, &find_size);
    if (find_result == MJSON_TOK_STRING) {
        out->resize(find_size, 0);
        mjson_get_string(request->params, request->params_len, param_pattern.get(), out->data(), find_size);
    } else {
        return false;
    }
    return true;
}

static uint8_t* GetResource(std::vector<char>& resource_name, size_t *resource_size) {
    uint64_t key = utils::StrHash(reinterpret_cast<const char *>(resource_name.data()));
    auto it = uploaded_resources.find(key);
    if (it == uploaded_resources.end()) {
        return nullptr;
    }

    if (resource_size) {
        *resource_size = it->second.size();
    }
    return it->second.data();
}

// Implementation of "get_serial_number" RPC.
// Returns JSON results with the key "serial_number" and the serial, as a string.
void GetSerialNumber(struct jsonrpc_request *request) {
    char serial_number_str[16];
    uint64_t serial_number = valiant::utils::GetUniqueID();
    for (int i = 0; i < 16; ++i) {
        uint8_t nibble = (serial_number >> (i * 4)) & 0xF;
        if (nibble < 10) {
            serial_number_str[15 - i] = nibble + '0';
        } else {
            serial_number_str[15 - i] = (nibble - 10) + 'a';
        }
    }
    jsonrpc_return_success(request, "{%Q:%.*Q}", "serial_number", 16, serial_number_str);
}

// Implements the "run_testconv1" RPC.
// Runs the simple "testconv1" model using the TPU.
// NOTE: The TPU power must be enabled for this RPC to succeed.
void RunTestConv1(struct jsonrpc_request *request) {
    if (!valiant::EdgeTpuTask::GetSingleton()->GetPower()) {
        jsonrpc_return_error(request, -1, "TPU power is not enabled", nullptr);
        return;
    }
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!valiant::testconv1::setup()) {
        jsonrpc_return_error(request, -1, "testconv1 setup failed", nullptr);
        return;
    }
    if (!valiant::testconv1::loop()) {
        jsonrpc_return_error(request, -1, "testconv1 loop failed", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{}");
}

// Implements the "set_tpu_power_state" RPC.
// Takes one parameter, "enable" -- a boolean indicating the state to set.
// Returns success or failure.
void SetTPUPowerState(struct jsonrpc_request *request) {
    bool enable;
    if (!JSONRPCGetBooleanParam(request, "enable", &enable)) {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    valiant::EdgeTpuTask::GetSingleton()->SetPower(enable);
    jsonrpc_return_success(request, "{}");
}

void BeginUploadResource(struct jsonrpc_request *request) {
    std::vector<char> resource_name;
    int resource_size;

    if (!JSONRPCGetStringParam(request, "name", &resource_name)) {
        jsonrpc_return_error(request, -1, "'name' missing", nullptr);
        return;
    }

    if (!JSONRPCGetIntegerParam(request, "size", &resource_size)) {
        jsonrpc_return_error(request, -1, "'size' missing", nullptr);
        return;
    }

    uploaded_resources.insert({utils::StrHash(reinterpret_cast<const char *>(resource_name.data())), std::vector<uint8_t>(resource_size)});

    jsonrpc_return_success(request, "{}");
}

void UploadResourceChunk(struct jsonrpc_request *request) {
    std::vector<char> resource_name;
    std::vector<char> resource_data;
    int offset;

    if (!JSONRPCGetStringParam(request, "name", &resource_name)) {
        jsonrpc_return_error(request, -1, "'name' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "offset", &offset)) {
        jsonrpc_return_error(request, -1, "'offset' missing", nullptr);
        return;
    }
    if (!JSONRPCGetStringParam(request, "data", &resource_data)) {
        jsonrpc_return_error(request, -1, "'data' missing", nullptr);
        return;
    }

    uint8_t* resource = GetResource(resource_name, nullptr);
    if (!resource) {
        jsonrpc_return_error(request, -1, "unknown resource", nullptr);
        return;
    }

    unsigned int bytes_to_decode = strlen(resource_data.data());
    size_t decoded_length = 0;
    mbedtls_base64_decode(nullptr, 0, &decoded_length, reinterpret_cast<unsigned char*>(resource_data.data()), bytes_to_decode);
    mbedtls_base64_decode(resource + offset, decoded_length, &decoded_length, reinterpret_cast<unsigned char*>(resource_data.data()), bytes_to_decode);

    jsonrpc_return_success(request, "{}");
}

void DeleteResource(struct jsonrpc_request *request) {
    std::vector<char> resource_name;

    if (!JSONRPCGetStringParam(request, "name", &resource_name)) {
        jsonrpc_return_error(request, -1, "'name' missing", nullptr);
        return;
    }

    uint64_t key = utils::StrHash(reinterpret_cast<const char*>(resource_name.data()));
    auto it = uploaded_resources.find(key);
    if (it == uploaded_resources.end()) {
        jsonrpc_return_error(request, -1, "unknown resource", nullptr);
        return;
    }

    uploaded_resources.erase(key);
    jsonrpc_return_success(request, "{}");
}

void RunDetectionModel(struct jsonrpc_request* request) {
    std::vector<char> model_resource_name, image_resource_name;
    int image_width, image_height, image_depth;

    if (!JSONRPCGetStringParam(request, "model_resource_name", &model_resource_name)) {
        jsonrpc_return_error(request, -1, "'model_resource_name' missing", nullptr);
        return;
    }
    if (!JSONRPCGetStringParam(request, "image_resource_name", &image_resource_name)) {
        jsonrpc_return_error(request, -1, "'image_resource_name' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_width", &image_width)) {
        jsonrpc_return_error(request, -1, "'image_width' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_height", &image_height)) {
        jsonrpc_return_error(request, -1, "'image_height' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_depth", &image_depth)) {
        jsonrpc_return_error(request, -1, "'image_depth' missing", nullptr);
        return;
    }

    size_t model_size;
    uint8_t* model_resource = GetResource(model_resource_name, &model_size);
    if (!model_resource || !model_size) {
        jsonrpc_return_error(request, -1, "missing model resource", nullptr);
        return;
    }
    uint8_t* image_resource = GetResource(image_resource_name, nullptr);
    if (!image_resource) {
        jsonrpc_return_error(request, -1, "missing image resource", nullptr);
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_resource);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        char buf[50];
        sprintf(buf, "model schema version unsupported: %ld", model->version());
        jsonrpc_return_error(request, -1, buf, nullptr);
        return;
    }


    tflite::MicroErrorReporter error_reporter;
    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice(PerformanceMode::kMax);
    if (!context) {
        jsonrpc_return_error(request, -1, "failed to open TPU", nullptr);
        return;
    }
    constexpr size_t kTensorArenaSize{8 * 1024 * 1024};

//    std::unique_ptr<uint8_t[]> tensor_arena(new(std::nothrow) uint8_t[kTensorArenaSize]);
//    if (!tensor_arena) {
//        jsonrpc_return_error(request, -1, "failed to allocate tensor arena", nullptr);
//        return;
//    }
    static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

    tflite::MicroMutableOpResolver</*tOpCount=*/3> resolver;
    resolver.AddDequantize();
    resolver.AddDetectionPostprocess();

    auto interpreter = tensorflow::MakeEdgeTpuInterpreter(model, context.get(), &resolver, &error_reporter,
                                                          tensor_arena, kTensorArenaSize);
    if (!interpreter) {
        jsonrpc_return_error(request, -1, "failed to make interpreter", nullptr);
        return;
    }

    auto* input_tensor = interpreter->input_tensor(0);
    auto* input_tensor_data = tflite::GetTensorData<uint8_t>(input_tensor);
    tensorflow::ImageDims tensor_dims = {
        input_tensor->dims->data[1],
        input_tensor->dims->data[2],
        input_tensor->dims->data[3]
    };
    auto preprocess_start = valiant::timer::micros();
    if (!tensorflow::ResizeImage({image_height, image_width, image_depth},
                                 image_resource, tensor_dims, input_tensor_data)) {
        jsonrpc_return_error(request, -1, "Failed to resize input image", nullptr);
        return;
    }
    auto preprocess_latency = valiant::timer::micros() - preprocess_start;

    // The first Invoke is slow due to model transfer. Run an Invoke
    // but ignore the results.
    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter", nullptr);
        return;
    }

    auto invoke_start = valiant::timer::micros();
    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter", nullptr);
        return;
    }
    auto invoke_latency = valiant::timer::micros() - invoke_start;

    // Return results and check on host side
    auto results = tensorflow::GetDetectionResults(interpreter.get(), 0.7, 3);
    if (results.empty()) {
        jsonrpc_return_error(request, -1, "no results above threshold", nullptr);
        return;
    }
    const auto& top_result = results.at(0);
    jsonrpc_return_success(request,
                           "{%Q: %d, %Q: %g, %Q: %g, %Q: %g, %Q: %g, %Q: %g, %Q:%d}",
                           "id", top_result.id,
                           "score", top_result.score,
                           "xmin", top_result.bbox.xmin,
                           "xmax", top_result.bbox.xmax,
                           "ymin", top_result.bbox.ymin,
                           "ymax", top_result.bbox.ymax,
                           "latency", (preprocess_latency + invoke_latency));
}

void RunClassificationModel(struct jsonrpc_request* request) {
    std::vector<char> model_resource_name, image_resource_name;
    int image_width, image_height, image_depth;

    if (!JSONRPCGetStringParam(request, "model_resource_name", &model_resource_name)) {
        jsonrpc_return_error(request, -1, "'model_resource_name' missing", nullptr);
        return;
    }
    if (!JSONRPCGetStringParam(request, "image_resource_name", &image_resource_name)) {
        jsonrpc_return_error(request, -1, "'image_resource_name' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_width", &image_width)) {
        jsonrpc_return_error(request, -1, "'image_width' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_height", &image_height)) {
        jsonrpc_return_error(request, -1, "'image_height' missing", nullptr);
        return;
    }
    if (!JSONRPCGetIntegerParam(request, "image_depth", &image_depth)) {
        jsonrpc_return_error(request, -1, "'image_depth' missing", nullptr);
        return;
    }

    uint8_t* model_resource = GetResource(model_resource_name, nullptr);
    if (!model_resource) {
        jsonrpc_return_error(request, -1, "missing model resource", nullptr);
        return;
    }
    uint8_t* image_resource = GetResource(image_resource_name, nullptr);
    if (!image_resource) {
        jsonrpc_return_error(request, -1, "missing image resource", nullptr);
        return;
    }

    const tflite::Model* model = tflite::GetModel(model_resource);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        jsonrpc_return_error(request, -1, "model schema version unsupported", nullptr);
        return;
    }

    tflite::MicroErrorReporter error_reporter;
    tflite::MicroMutableOpResolver<1> resolver;
    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        jsonrpc_return_error(request, -1, "failed to open TPU", nullptr);
        return;
    }

    constexpr size_t kTensorArenaSize = 1 * 1024 * 1024;
    // Use std::nothrow to detect out-of-memory condition specifically in the test lib and
    // prevent a silent crash.
    std::unique_ptr<uint8_t[]> tensor_arena(new(std::nothrow) uint8_t[kTensorArenaSize]);
    if (!tensor_arena) {
        jsonrpc_return_error(request, -1, "failed to allocate tensor arena", nullptr);
        return;
    }
    auto interpreter = tensorflow::MakeEdgeTpuInterpreter(model, context.get(), &resolver, &error_reporter,
                                                          tensor_arena.get(), kTensorArenaSize);
    if (!interpreter) {
        jsonrpc_return_error(request, -1, "failed to make interpreter", nullptr);
        return;
    }

    auto* input_tensor = interpreter->input_tensor(0);
    bool needs_preprocessing = tensorflow::ClassificationInputNeedsPreprocessing(*input_tensor);
    uint32_t preprocess_latency = 0;
    if (needs_preprocessing) {
        uint32_t preprocess_start = valiant::timer::micros();
        if (!tensorflow::ClassificationPreprocess(input_tensor)) {
            jsonrpc_return_error(request, -1, "input preprocessing failed", nullptr);
            return;
        }
        uint32_t preprocess_end = valiant::timer::micros();
        preprocess_latency = preprocess_end - preprocess_start;
    }

    // Resize into input tensor
    tensorflow::ImageDims input_tensor_dims = {
        input_tensor->dims->data[1],
        input_tensor->dims->data[2],
        input_tensor->dims->data[3]
    };
    if (!tensorflow::ResizeImage(
        {image_height, image_width, image_depth}, image_resource,
        input_tensor_dims, tflite::GetTensorData<uint8_t>(input_tensor))) {
        jsonrpc_return_error(request, -1, "failed to resize input", nullptr);
        return;
    }

    // The first Invoke is slow due to model transfer. Run an Invoke
    // but ignore the results.
    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter", nullptr);
        return;
    }

    uint32_t start = valiant::timer::micros();
    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter", nullptr);
        return;
    }
    uint32_t end = valiant::timer::micros();
    uint32_t latency = end - start;

    // Return results and check on host side
    auto results = tensorflow::GetClassificationResults(interpreter.get(), 0.0f, 1);
    if (results.empty()) {
        jsonrpc_return_error(request, -1, "no results above threshold", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q:%d, %Q:%g, %Q:%d}", "id", results[0].id, "score", results[0].score, "latency", latency + preprocess_latency);
}

void StartM4(struct jsonrpc_request *request) {
    auto* ipc = static_cast<IPCM7*>(IPC::GetSingleton());
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

void GetTemperature(struct jsonrpc_request *request) {
    int sensor_num;
    float temperature;
    if (!JSONRPCGetIntegerParam(request, "sensor", &sensor_num)) {
        jsonrpc_return_error(request, -1, "'sensor' missing", nullptr);
        return;
    }

    auto sensor = static_cast<valiant::tempsense::TempSensor>(sensor_num);
    if(sensor >= valiant::tempsense::TempSensor::kSensorCount) {
        jsonrpc_return_error(request, -1, "Invalid temperature sensor", nullptr);
        return;
    }

    temperature = valiant::tempsense::GetTemperature(sensor);
    jsonrpc_return_success(request, "{%Q:%g}", "temperature", temperature);
}

// Implements the "capture_test_pattern" RPC.
// Configures the sensor to test pattern mode, and captures via trigger.
// Returns success if the test pattern has the expected data, failure otherwise.
void CaptureTestPattern(struct jsonrpc_request *request) {
    if (!valiant::CameraTask::GetSingleton()->SetPower(true)) {
        valiant::CameraTask::GetSingleton()->SetPower(false);
        jsonrpc_return_error(request, -1, "unable to detect camera", nullptr);
        return;
    }
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::TRIGGER);
    valiant::CameraTask::GetSingleton()->SetTestPattern(
        valiant::camera::TestPattern::WALKING_ONES);

    valiant::CameraTask::GetSingleton()->Trigger();

    uint8_t* buffer = nullptr;
    int index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
    uint8_t expected = 0;
    bool success = true;
    for (unsigned int i = 0; i < valiant::CameraTask::kWidth * valiant::CameraTask::kHeight; ++i) {
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
    if (success) {
        jsonrpc_return_success(request, "{}");
    } else {
        jsonrpc_return_error(request, -1, "camera test pattern mismatch", nullptr);
    }
    valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    valiant::CameraTask::GetSingleton()->SetPower(false);
}

// Implements the "capture_audio" RPC.
// Attempts to capture 1 second of audio.
// Returns success, with a parameter "data" containing the captured audio in base64 (or failure).
// The audio captured is 32-bit signed PCM @ 16000Hz.
void CaptureAudio(struct jsonrpc_request *request) {
    struct AudioParams {
        SemaphoreHandle_t sema;
        int count;
        uint32_t* audio_data;
    };
    AudioParams params;
    std::vector<uint32_t> audio_data(16000, 0xA5A5A5A5);
    size_t audio_data_size = sizeof(uint32_t) * audio_data.size();
    params.sema = xSemaphoreCreateBinary();
    params.count = 0;
    params.audio_data = audio_data.data();
    valiant::AudioTask::GetSingleton()->SetCallback([](void *param) {
        auto* params = reinterpret_cast<AudioParams*>(param);
        // If this is the second time we hit the callback, return nothing for the buffer
        // to terminate recording.
        // Otherwise, we feed the original buffer back in. This should give us a clean recording
        // of one second, without any sort of pops or glitches that happen when the mic starts.
        if (params->count == 1) {
            BaseType_t reschedule = pdFALSE;
            xSemaphoreGiveFromISR(params->sema, &reschedule);
            portYIELD_FROM_ISR(reschedule);
            return reinterpret_cast<uint32_t*>(0);
        } else {
            params->count++;
            return params->audio_data;
        }
    }, &params);

    valiant::AudioTask::GetSingleton()->SetPower(true);
    valiant::AudioTask::GetSingleton()->SetBuffer(audio_data.data(), audio_data_size);
    valiant::AudioTask::GetSingleton()->Enable();
    BaseType_t ret = xSemaphoreTake(params.sema, pdMS_TO_TICKS(3000));
    valiant::AudioTask::GetSingleton()->Disable();
    valiant::AudioTask::GetSingleton()->SetPower(false);
    vSemaphoreDelete(params.sema);

    if (ret != pdTRUE) {
        jsonrpc_return_error(request, -1, "semaphore timed out", nullptr);
    } else {
        size_t encoded_length = 0;
        mbedtls_base64_encode(nullptr, 0, &encoded_length, reinterpret_cast<uint8_t*>(audio_data.data()), audio_data_size);
        std::vector<uint8_t> encoded_data(encoded_length);
        mbedtls_base64_encode(encoded_data.data(), encoded_length, &encoded_length, reinterpret_cast<uint8_t*>(audio_data.data()), audio_data_size);
        jsonrpc_return_success(request, "{%Q:%.*Q}", "data", encoded_length, encoded_data.data());
    }
}

void WifiSetAntenna(struct jsonrpc_request *request) {
    double antenna_double;
    int antenna;
    const char *antenna_param_pattern = "$[0].antenna";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, antenna_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, antenna_param_pattern, &antenna_double);
        antenna = static_cast<int>(antenna_double);
    } else {
        jsonrpc_return_error(request, -1, "'antenna' missing", nullptr);
        return;
    }

    enum Antenna {
        kInternal = 0,
        kExternal = 1,
    };


    switch (antenna) {
        case kInternal:
            gpio::SetGpio(gpio::Gpio::kAntennaSelect, false);
            break;
        case kExternal:
            gpio::SetGpio(gpio::Gpio::kAntennaSelect, true);
            break;
        default:
            jsonrpc_return_error(request, -1, "invalid antenna selection", nullptr);
            return;
    }
    jsonrpc_return_success(request, "{}");
}

}  // namespace testlib
}  // namespace valiant
