#include "libs/YAMNet/yamnet.h"

#include "libs/base/filesystem.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace coral::micro {
namespace yamnet {

namespace {
constexpr int kTensorArenaSize = 1 * 1024 * 1024;
STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

std::unique_ptr<tflite::MicroInterpreter> interpreter;
std::shared_ptr<coral::micro::EdgeTpuContext> edgetpu_context;
std::shared_ptr<tflite::MicroErrorReporter> error_reporter;
std::shared_ptr<TfLiteTensor> input_tensor;

constexpr float kThreshold = 0.05;
constexpr int kTopK = 5;

void PrintOutput(const std::shared_ptr<std::vector<tensorflow::Class>>& output) {
    printf("YAMNet Results:\r\n");
    for (const auto& classes : *output) {
        printf("%d: %f\r\n", classes.id, classes.score);
    }
    printf("\r\n");
}

#ifdef YAMNET_CPU
constexpr char kModelName[] = "/models/yamnet.tflite";
#else
constexpr char kModelName[] = "/models/yamnet_edgetpu.tflite";
#endif
}  // namespace

std::shared_ptr<TfLiteTensor> input() {
    return input_tensor;
}

bool setup() {
    TfLiteStatus allocate_status;
    error_reporter = std::make_shared<tflite::MicroErrorReporter>();
    TF_LITE_REPORT_ERROR(error_reporter.get(), "YAMNet!");

    std::vector<uint8_t> yamnet_edgetpu_tflite;
    if (!coral::micro::filesystem::ReadFile(kModelName, &yamnet_edgetpu_tflite)) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Failed to load model!");
        return false;
    }

    std::vector<uint8_t> yamnet_test_input_bin;
    if (!coral::micro::filesystem::ReadFile("/models/yamnet_test_input.bin",
                                       &yamnet_test_input_bin)) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Failed to load test input!");
        return false;
    }

    const auto *model = tflite::GetModel(yamnet_edgetpu_tflite.data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter.get(),
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    edgetpu_context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(coral::micro::PerformanceMode::kMax);
    if (!edgetpu_context) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Failed to get TPU context");
        return false;
    }

#ifdef YAMNET_CPU
    constexpr int kNumTensorOps = 9;
    auto resolver = std::make_unique<tflite::MicroMutableOpResolver<kNumTensorOps>>();
    resolver->AddQuantize();
    resolver->AddDequantize();
    resolver->AddReshape();
    resolver->AddSplit();
    resolver->AddConv2D();
    resolver->AddDepthwiseConv2D();
    resolver->AddLogistic();
    resolver->AddMean();
    resolver->AddFullyConnected();

    interpreter = std::make_unique<tflite::MicroInterpreter>(model, *resolver.get(),
        tensor_arena, kTensorArenaSize, error_reporter.get());
#else
    // Three operations are required, EdgeTPU is added when the Interpeter
    // is created.
    constexpr int kNumTensorOps = 3;
    auto resolver = std::make_unique<tflite::MicroMutableOpResolver<kNumTensorOps>>();
    resolver->AddQuantize();
    resolver->AddDequantize();
    resolver->AddCustom(kCustomOp, RegisterCustomOp());

    interpreter = std::make_unique<tflite::MicroInterpreter>(
        model, *resolver, tensor_arena, kTensorArenaSize, error_reporter.get());
#endif

    allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "AllocateTensors failed.");
        return false;
    }

    input_tensor.reset(interpreter->input(0));

    if (input_tensor->bytes != yamnet_test_input_bin.size()) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Input tensor length doesn't match canned input\r\n");
        return false;
    }
    memcpy(tflite::GetTensorData<float>(input_tensor.get()), yamnet_test_input_bin.data(), yamnet_test_input_bin.size());
    return true;

}

bool loop(std::shared_ptr<std::vector<tensorflow::Class>> output) {
    return loop(std::move(output), true);
}

bool loop(std::shared_ptr<std::vector<tensorflow::Class>> output, bool print) {
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return false;
    }
    output = std::make_shared<std::vector<tensorflow::Class>>(tensorflow::GetClassificationResults(interpreter.get(), kThreshold, kTopK));

    if (print) {
        PrintOutput(output);
    }
    return true;
}

} // namespace yamnet
} // namespace coral::micro

