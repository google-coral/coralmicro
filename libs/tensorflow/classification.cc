#include "libs/tensorflow/classification.h"
#include "libs/tensorflow/utils.h"

#include <queue>
#include <vector>

namespace valiant {
namespace tensorflow {
namespace {
// Defines a comparator which allows us to rank Class based on their score and
// id.
struct ClassComparator {
  bool operator()(const Class& lhs, const Class& rhs) const {
    return std::tie(lhs.score, lhs.id) > std::tie(rhs.score, rhs.id);
  }
};
}  // namespace

std::vector<Class> GetClassificationResults(
                    const float *scores, ssize_t scores_count,
                    float threshold, size_t top_k) {
    std::priority_queue<Class, std::vector<Class>, ClassComparator> q;
    for (int i = 0; i < scores_count; ++i) {
        if (scores[i] < threshold) continue;
        q.push(Class{i, scores[i]});
        if (q.size() > top_k) q.pop();
    }

    std::vector<Class> ret;
    while (!q.empty()) {
        ret.push_back(q.top());
        q.pop();
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}

std::vector<Class> GetClassificationResults(
                    tflite::MicroInterpreter* interpreter,
                    float threshold, size_t top_k) {
    auto tensor = interpreter->output_tensor(0);
    if (tensor->type == kTfLiteUInt8 || tensor->type == kTfLiteInt8) {
        auto scores = DequantizeTensor<float>(tensor);
        return GetClassificationResults(scores.data(), scores.size(), threshold, top_k);
    } else if (tensor->type == kTfLiteFloat32) {
        auto scores = tflite::GetTensorData<float>(tensor);
        return GetClassificationResults(scores, TensorSize(tensor), threshold, top_k);
    } else {
        assert(false);
    }
}

}  // namespace tensorflow
}  // namespace valiant