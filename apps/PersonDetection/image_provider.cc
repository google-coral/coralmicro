#include "apps/PersonDetection/test_helpers.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/flatbuffers/include/flatbuffers/flatbuffers.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/image_provider.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/model_settings.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/schema/schema_generated.h"

#include <cstring>
#include <memory>

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {

    auto unsigned_image_data = std::make_unique<uint8_t[]>(image_width * image_height);
    valiant::camera::FrameFormat fmt;
    fmt.width = image_width;
    fmt.height = image_height;
    fmt.fmt = valiant::camera::Format::Y8;
    fmt.preserve_ratio = false;
    fmt.buffer = unsigned_image_data.get();
    bool ret = valiant::CameraTask::GetFrame({fmt});
    for (int i = 0; i < image_width * image_height; ++i) {
        image_data[i] = unsigned_image_data[i] - 128;
    }
    return ret ? kTfLiteOk : kTfLiteError;
}
