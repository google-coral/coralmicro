#include "libs/YAMNet/yamnet.h"

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/base/timer.h"
#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"
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

#ifdef YAMNET_CPU
constexpr int kNumTensorOps = 9;
constexpr char kModelName[] = "/models/yamnet.tflite";
#else
constexpr int kNumTensorOps = 3;
constexpr char kModelName[] = "/models/yamnet_edgetpu.tflite";
#endif

std::unique_ptr<tflite::MicroMutableOpResolver<kNumTensorOps>> resolver;

std::shared_ptr<int16_t[]> audio_data = nullptr;

std::unique_ptr<FrontendState> frontend_state = nullptr;

std::shared_ptr<uint16_t[]> feature_buffer = nullptr;

std::vector<uint8_t> yamnet_edgetpu_tflite;

constexpr float kThreshold = 0.3;
constexpr int kTopK = 5;

constexpr float kExpectedSpectraMax = 3.5f;

void PrintOutput(const std::vector<tensorflow::Class>& output) {
    printf("YAMNet Results:\r\n");
    for (const auto& classes : output) {
        printf("%d: %f\r\n", classes.id, classes.score);
    }
    printf("\r\n");
}

TfLiteStatus PrepareFrontEnd() {
    FrontendConfig config;
    config.window.size_ms = kFeatureSliceDurationMs;
    config.window.step_size_ms = kFeatureSliceStrideMs;
    config.filterbank.num_channels = kFeatureSliceSize;
    config.filterbank.lower_band_limit = 125.0;
    config.filterbank.upper_band_limit = 7500.0;
    config.noise_reduction.smoothing_bits = 10;
    config.noise_reduction.even_smoothing = 0.025;
    config.noise_reduction.odd_smoothing = 0.06;
    config.noise_reduction.min_signal_remaining =
        1.0;  // Use 1.0 to disable reduction.
    config.pcan_gain_control.enable_pcan = 0;
    config.pcan_gain_control.strength = 0.95;
    config.pcan_gain_control.offset = 80.0;
    config.pcan_gain_control.gain_bits = 21;
    config.log_scale.enable_log = 1;
    config.log_scale.scale_shift = 6;

    frontend_state = std::make_unique<FrontendState>();
    CHECK(frontend_state);
    if (!FrontendPopulateState(&config, frontend_state.get(), kSampleRate)) {
        TF_LITE_REPORT_ERROR(error_reporter.get(),
                             "FrontendPopulateState() failed");
        return kTfLiteError;
    }
    CHECK(frontend_state);
    return kTfLiteOk;
}
}  // namespace

std::shared_ptr<int16_t[]> audio_input() { return audio_data; }

bool setup() {
    TfLiteStatus allocate_status;
    error_reporter = std::make_shared<tflite::MicroErrorReporter>();
    TF_LITE_REPORT_ERROR(error_reporter.get(), "YAMNet!");

    if (!coral::micro::filesystem::ReadFile(kModelName,
                                            &yamnet_edgetpu_tflite)) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Failed to load model!");
        return false;
    }

    const auto* model = tflite::GetModel(yamnet_edgetpu_tflite.data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter.get(),
                             "Model schema version is %d, supported is %d",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

#ifdef YAMNET_CPU
    resolver =
        std::make_unique<tflite::MicroMutableOpResolver<kNumTensorOps>>();
    resolver->AddQuantize();
    resolver->AddDequantize();
    resolver->AddReshape();
    resolver->AddSplit();
    resolver->AddConv2D();
    resolver->AddDepthwiseConv2D();
    resolver->AddLogistic();
    resolver->AddMean();
    resolver->AddFullyConnected();

    interpreter = std::make_unique<tflite::MicroInterpreter>(
        model, *resolver, tensor_arena, kTensorArenaSize, error_reporter.get());
#else
    // Three operations are required, EdgeTPU is added when the Interpeter
    // is created.
    resolver =
        std::make_unique<tflite::MicroMutableOpResolver<kNumTensorOps>>();
    resolver->AddQuantize();
    resolver->AddDequantize();
    resolver->AddCustom(kCustomOp, RegisterCustomOp());

    edgetpu_context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(
        coral::micro::PerformanceMode::kMax);
    if (!edgetpu_context) {
        TF_LITE_REPORT_ERROR(error_reporter.get(), "Failed to get TPU context");
        return false;
    }

    interpreter = std::make_unique<tflite::MicroInterpreter>(
        model, *resolver, tensor_arena, kTensorArenaSize, error_reporter.get());
#endif

    allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter.get(),
                             "AllocateTensors failed.\r\n");
        return false;
    }

    input_tensor.reset(interpreter->input(0));

    audio_data = std::shared_ptr<int16_t[]>(new int16_t[kAudioSize]);
    feature_buffer =
        std::shared_ptr<uint16_t[]>(new uint16_t[kFeatureElementCount]);

    if (PrepareFrontEnd() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter.get(),
                             "Prepare Frontend failed.\r\n");
        return false;
    }

    return true;
}

std::optional<const std::vector<tensorflow::Class>> loop(bool print) {
    // TODO(michaelbrooks): Properly slice the data so that we don't need to
    // re-run the frontend on windows we've already processed.
    uint32_t process_start = coral::micro::timer::millis();
    int num_samples_remaining = kAudioSize;
    int16_t* audio = audio_data.get();
    int count = 0;
    while (num_samples_remaining > 0) {
        size_t num_samples_read;
        struct FrontendOutput output =
            FrontendProcessSamples(frontend_state.get(), audio,
                                   num_samples_remaining, &num_samples_read);
        audio += num_samples_read;
        num_samples_remaining -= num_samples_read;
        if (output.values != NULL) {
            for (size_t i = 0; i < output.size; ++i) {
                feature_buffer[count++] = output.values[i];
            }
        }
    }

    float* input = tflite::GetTensorData<float>(input_tensor.get());

    // Determine the offset and scalar based on the calculated data.
    // TODO(michaelbrooks): This likely isn't needed, the values are always
    // around the same. Can likely hard code.
    float max = 0;
    float min = 10000.0f;
    for (int i = 0; i < kFeatureElementCount; ++i) {
        input[i] = static_cast<float>(feature_buffer[i]);
        if (input[i] > max) {
            max = input[i];
        }
        if (input[i] < min) {
            min = input[i];
        }
    }

    int offset = (max + min) / 2;
    float scalar = (kExpectedSpectraMax / (max - offset));
    for (int i = 0; i < kFeatureElementCount; ++i) {
        input[i] = (input[i] - offset) * scalar;
    }

    uint32_t invoke_start = coral::micro::timer::millis();
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return std::nullopt;
    }

    const std::vector<tensorflow::Class> output =
        tensorflow::GetClassificationResults(interpreter.get(), kThreshold,
                                             kTopK);
    uint32_t process_end = coral::micro::timer::millis();
    if (print) {
        printf("Ran YAMNet + Frontend in %ld ms\r\n",
               process_end - process_start);
        printf("Ran Invoke in %ld ms\r\n", process_end - invoke_start);
        PrintOutput(output);
    }
    return output;
}

}  // namespace yamnet
}  // namespace coral::micro
