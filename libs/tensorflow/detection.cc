#include "libs/tensorflow/detection.h"

#include <cmath>
#include <queue>

namespace valiant {
namespace tensorflow {

namespace {
struct ObjectComparator {
  bool operator()(const Object& lhs, const Object& rhs) const {
    return std::tie(lhs.score, lhs.id) > std::tie(rhs.score, rhs.id);
  }
};
}  // namespace

std::vector<Object> GetDetectionResults(
    const float *bboxes, const float *ids,
    const float *scores, size_t count,
    float threshold, size_t top_k) {
    std::priority_queue<Object, std::vector<Object>, ObjectComparator> q;

    for (unsigned int i = 0; i < count; ++i) {
        const int id = std::round(ids[i]);
        const float score = scores[i];
        const float ymin = std::max(0.0f, bboxes[4 * i]);
        const float xmin = std::max(0.0f, bboxes[4 * i + 1]);
        const float ymax = std::max(0.0f, bboxes[4 * i + 2]);
        const float xmax = std::max(0.0f, bboxes[4 * i + 3]);
        if (score < threshold) {
            continue;
        }
        q.push(Object{id, score, BBox<float>{ymin, xmin, ymax, xmax}});
        if (q.size() > top_k) {
            q.pop();
        }
    }

    std::vector<Object> ret;
    ret.reserve(q.size());
    while (!q.empty()) {
        ret.push_back(q.top());
        q.pop();
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}

std::vector<Object> GetDetectionResults(
    tflite::MicroInterpreter* interpreter,
    float threshold,
    size_t top_k) {
    const float *bboxes;
    const float *ids;
    const float *scores;
    const float *count;
    if (interpreter->outputs().size() != 4) {
        printf("Output size mismatch\r\n");
        return std::vector<Object>();
    }

    bboxes = tflite::GetTensorData<float>(interpreter->output_tensor(0));
    ids = tflite::GetTensorData<float>(interpreter->output_tensor(1));
    scores = tflite::GetTensorData<float>(interpreter->output_tensor(2));
    count = tflite::GetTensorData<float>(interpreter->output_tensor(3));

    return GetDetectionResults(bboxes, ids, scores, static_cast<size_t>(count[0]),
                               threshold, top_k);
}

}  // namespace tensorflow
}  // namespace valiant
