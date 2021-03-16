#include "third_party/freertos_kernel/include/projdefs.h"
#include "libs/base/tasks_m7.h"
#include "libs/posenet/posenet_decoder_op.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tensorflow/posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu.h"
#include "libs/tensorflow/posenet_test_input.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"

// Run Tensorflow's DebugLog to the debug console.
extern "C" void DebugLog(const char *s) {
    DbgConsole_Printf(s);
}

namespace {
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;

const int kModelArenaSize = 1 * 1024 * 1024;
const int kExtraArenaSize = 1 * 1024 * 1024;
const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
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

void PrintOutput() {
    float *keypoints = tflite::GetTensorData<float>(interpreter->output(0));
    float *keypoints_scores = tflite::GetTensorData<float>(interpreter->output(1));
    float *pose_scores = tflite::GetTensorData<float>(interpreter->output(2));
    float *num_poses = tflite::GetTensorData<float>(interpreter->output(3));

    int poses_count = static_cast<int>(num_poses[0]);
    for (int i = 0; i < poses_count; ++i) {
        float pose_score = pose_scores[i];
        if (pose_score < 0.4) {
            continue;
        }
        printf("Pose %d -- score: %f\r\n", i, pose_score);
        float *pose_keypoints = keypoints + (i * 17 * 2);
        float *keypoint_scores = keypoints_scores + (i * 17);
        for (int j = 0; j < 17; ++j) {
            float *point = pose_keypoints + (j * 2);
            float y = point[0];
            float x = point[1];
            float score = keypoint_scores[j];

            printf("Keypoint %s -- x=%f,y=%f,score=%f\r\n", KeypointTypes[j], x, y, score);
        }
    }
}

bool loop() {
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return false;
    }
    PrintOutput();
    return true;
}

static bool setup() {
    TfLiteStatus allocate_status;
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    TF_LITE_REPORT_ERROR(error_reporter, "Posenet!");

    model = tflite::GetModel(posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model schema version is %d, supported is %d",
            model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::AllOpsResolver resolver;
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

    input = interpreter->input(0);

    if (input->bytes != posenet_test_input_bin_len) {
        printf("Input tensor length doesn't match canned input\r\n");
        return false;
    }
    memcpy(input->data.uint8, posenet_test_input_bin, posenet_test_input_bin_len);
    return true;
}

extern "C" void app_main(void *param) {
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();
    setup();
    loop();
    while (true) { taskYIELD(); }
}
