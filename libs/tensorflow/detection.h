#ifndef LIBS_TENSORFLOW_DETECTION_H_
#define LIBS_TENSORFLOW_DETECTION_H_

#include <limits>
#include <vector>

#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

namespace coral::micro {
namespace tensorflow {

template <typename T>
struct BBox {
    T ymin;
    T xmin;
    T ymax;
    T xmax;
};

struct Object {
    int id;
    float score;
    BBox<float> bbox;
};

std::vector<Object> GetDetectionResults(
    const float* bboxes, const float* ids, const float* scores, size_t count,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

std::vector<Object> GetDetectionResults(
    tflite::MicroInterpreter* interpreter,
    float threshold = -std::numeric_limits<float>::infinity(),
    size_t top_k = std::numeric_limits<size_t>::max());

}  // namespace tensorflow
}  // namespace coral::micro

#endif  // LIBS_TENSORFLOW_DETECTION_H_
