#include "libs/base/utils.h"
#include "libs/base/ipc_m7.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/testlib/test_lib.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/base64.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/schema/schema_generated.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"

#include <array>
#include <map>

// Map for containing uploaded resources.
// Key is the output of StrHash with the resource name as the parameter.
static std::map<uint64_t, std::vector<uint8_t>> uploaded_resources;

namespace valiant {
namespace testlib {

static std::unique_ptr<char> JSONRPCCreateParamFormatString(const char *param_name) {
    const char *param_format = "$[0].%s";
    // +1 for null terminator.
    int param_pattern_len = snprintf(nullptr, 0, param_format, param_name) + 1;
    std::unique_ptr<char> param_pattern(new char[param_pattern_len]);
    snprintf(param_pattern.get(), param_pattern_len, param_format, param_name);
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

    uint64_t key = utils::StrHash(reinterpret_cast<const char *>(resource_name.data()));
    auto it = uploaded_resources.find(key);
    if (it == uploaded_resources.end()) {
        jsonrpc_return_error(request, -1, "unknown resource", nullptr);
        return;
    }

    uploaded_resources.erase(key);
    jsonrpc_return_success(request, "{}");
}

void RunClassificationModel(struct jsonrpc_request *request) {
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
    uint8_t* image_resource = GetResource(image_resource_name, nullptr);
    if (!model_resource) {
        jsonrpc_return_error(request, -1, "missing model resource", nullptr);
        return;
    }
    if (!image_resource) {
        jsonrpc_return_error(request, -1, "missing image resource", nullptr);
        return;
    }

    const tflite::Model *model = tflite::GetModel(model_resource);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        jsonrpc_return_error(request, -1, "model schema version unsupported", nullptr);
        return;
    }

    std::unique_ptr<tflite::MicroErrorReporter> error_reporter(new tflite::MicroErrorReporter());
    std::unique_ptr<tflite::MicroMutableOpResolver<1>> resolver(new tflite::MicroMutableOpResolver<1>);
    std::shared_ptr<EdgeTpuContext> context = EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!context) {
        jsonrpc_return_error(request, -1, "failed to open TPU", nullptr);
        return;
    }

    constexpr size_t kTensorArenaSize = 1 * 1024 * 1024;
    std::unique_ptr<uint8_t[]> tensor_arena(new uint8_t[kTensorArenaSize]);
    if (!tensor_arena) {
        jsonrpc_return_error(request, -1, "failed to allocate tensor arena", nullptr);
        return;
    }
    auto interpreter = tensorflow::MakeEdgeTpuInterpreter(model, context.get(), resolver.get(), error_reporter.get(), tensor_arena.get(), kTensorArenaSize);
    if (!interpreter) {
        jsonrpc_return_error(request, -1, "failed to make interpreter", nullptr);
        return;
    }

    auto* input_tensor = interpreter->input_tensor(0);
    bool needs_preprocessing = tensorflow::ClassificationInputNeedsPreprocessing(*input_tensor);
    if (needs_preprocessing) {
        jsonrpc_return_error(request, -1, "input needs preprocessing, not supported", nullptr);
        return;
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

    // Invoke
    if (interpreter->Invoke() != kTfLiteOk) {
        jsonrpc_return_error(request, -1, "failed to invoke interpreter", nullptr);
        return;
    }
    // Return results and check on host side
    auto results = tensorflow::GetClassificationResults(interpreter.get(), 0.0f, 1);
    if (results.size() < 1) {
        jsonrpc_return_error(request, -1, "no results above threshold", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{%Q:%d, %Q:%g}", "id", results[0].id, "score", results[0].score);
}

void StartM4(struct jsonrpc_request *request) {
    IPCM7* ipc = static_cast<IPCM7*>(IPC::GetSingleton());
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

}  // namespace testlib
}  // namespace valiant
