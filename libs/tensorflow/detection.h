#ifndef _LIBS_TENSORFLOW_DETECTION_H_
#define _LIBS_TENSORFLOW_DETECTION_H_

#include "libs/tensorflow/utils.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include <limits>
#include <vector>

namespace valiant {
namespace tensorflow {

struct Object {
    int id;
    float score;
    BBox<float> bbox;
};

std::vector<Object> GetDetectionResults(
    const float *bboxes, const float *ids,
    const float *scores, size_t count,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

std::vector<Object> GetDetectionResults(
    tflite::MicroInterpreter* interpreter,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

}  // namespace tensorflow
}  // namespace valiant

#endif  // _LIBS_TENSORFLOW_DETECTION_H_
