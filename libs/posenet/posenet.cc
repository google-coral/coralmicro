#include "libs/posenet/posenet.h"

#include "libs/base/filesystem.h"
#include "libs/posenet/posenet_decoder_op.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace valiant {
namespace posenet {

namespace {
static tflite::ErrorReporter *error_reporter = nullptr;
static const tflite::Model *model = nullptr;
static tflite::MicroInterpreter *interpreter = nullptr;
static TfLiteTensor *input_tensor = nullptr;

static const int kModelArenaSize = 1 * 1024 * 1024;
static const int kExtraArenaSize = 1 * 1024 * 1024;
static const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

static std::vector<uint8_t> posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu_tflite;
static std::vector<uint8_t> posenet_test_input_bin;
}  // namespace

const char* const KeypointTypes[] = {
    "NOSE",
    "LEFT_EYE",
    "RIGHT_EYE",
    "LEFT_EAR",
    "RIGHT_EAR",
    "LEFT_SHOULDER",
    "RIGHT_SHOULDER",
    "LEFT_ELBOW",
    "RIGHT_ELBOW",
    "LEFT_WRIST",
    "RIGHT_WRIST",
    "LEFT_HIP",
    "RIGHT_HIP",
    "LEFT_KNEE",
    "RIGHT_KNEE",
    "LEFT_ANKLE",
    "RIGHT_ANKLE",
};

static void PrintOutput(const Output& output) {
    int poses_count = output.num_poses;
    printf("Poses: %d\r\n", poses_count);
    for (int i = 0; i < poses_count; ++i) {
        float pose_score = output.poses[i].score;
        if (pose_score < 0.4) {
            continue;
        }
        printf("Pose %d -- score: %f\r\n", i, pose_score);
        for (int j = 0; j < kKeypoints; ++j) {
            float y = output.poses[i].keypoints[j].y;
            float x = output.poses[i].keypoints[j].x;
            float score = output.poses[i].keypoints[j].score;

            printf("Keypoint %s -- x=%f,y=%f,score=%f\r\n", KeypointTypes[j], x, y, score);
        }
    }
}

TfLiteTensor* input() {
    return input_tensor;
}

bool loop(Output* output) {
    return loop(output, true);
}

bool loop(Output* output, bool print) {
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return false;
    }

    if (output) {
        memset(output, 0, sizeof(Output));
        float *keypoints = tflite::GetTensorData<float>(interpreter->output(0));
        float *keypoints_scores = tflite::GetTensorData<float>(interpreter->output(1));
        float *pose_scores = tflite::GetTensorData<float>(interpreter->output(2));
        float *num_poses = tflite::GetTensorData<float>(interpreter->output(3));
        output->num_poses = static_cast<int>(num_poses[0]);
        for (int i = 0; i < output->num_poses; ++i) {
            output->poses[i].score = pose_scores[i];
            float *pose_keypoints = keypoints + (i * kKeypoints * 2);
            float *keypoint_scores = keypoints_scores + (i * kKeypoints);
            for (int j = 0; j < kKeypoints; ++j) {
            float *point = pose_keypoints + (j * 2);
                output->poses[i].keypoints[j].x = point[1];
                output->poses[i].keypoints[j].y = point[0];
                output->poses[i].keypoints[j].score = keypoint_scores[j];
            }
        }

        if (print) {
            PrintOutput(*output);
        }
    }

    return true;
}

bool setup() {
    TfLiteStatus allocate_status;
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    TF_LITE_REPORT_ERROR(error_reporter, "Posenet!");

    if (!valiant::filesystem::ReadFile(
            "/models/posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu.tflite",
            &posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu_tflite)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load model!");
        return false;
    }

    if (!valiant::filesystem::ReadFile("/models/posenet_test_input.bin",
                                       &posenet_test_input_bin)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Failed to load test input!");
        return false;
    }

    model = tflite::GetModel(posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu_tflite.data());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::MicroMutableOpResolver<2> resolver;
    resolver.AddCustom("edgetpu-custom-op", valiant::RegisterCustomOp());
    resolver.AddCustom(coral::kPosenetDecoderOp, coral::RegisterPosenetDecoderOp());
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        return false;
    }

    input_tensor = interpreter->input(0);

    if (input_tensor->bytes != posenet_test_input_bin.size()) {
        TF_LITE_REPORT_ERROR(error_reporter, "Input tensor length doesn't match canned input\r\n");
        return false;
    }
    memcpy(input_tensor->data.uint8, posenet_test_input_bin.data(), posenet_test_input_bin.size());
    return true;
}

}  // namespace posenet
}  // namespace valiant
