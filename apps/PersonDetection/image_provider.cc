#include "apps/PersonDetection/test_helpers.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/flatbuffers/include/flatbuffers/flatbuffers.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/image_provider.h"
#include "third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/model_settings.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/schema/schema_generated.h"

#include <cstring>
#include <memory>

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, uint8_t* image_data) {

    valiant::camera::FrameFormat fmt;
    fmt.width = image_width;
    fmt.height = image_height;
    fmt.fmt = valiant::camera::Format::Y8;
    fmt.preserve_ratio = false;
    bool ret = valiant::CameraTask::GetFrame(fmt, image_data);
    return ret ? kTfLiteOk : kTfLiteError;
}
