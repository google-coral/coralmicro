#include "apps/OOBE/oobe_json.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace valiant {
namespace oobe {
namespace {
template<typename... T>
void append(std::vector<char>* v, const char* format, T... args) {
  const int size = std::snprintf(nullptr, 0, format, args...) + 1;
  v->resize(v->size() + size);
  std::snprintf(&(*v)[v->size() - size], size, format, args...);
  v->resize(v->size() - 1);  // remove null terminator
}
void append(std::vector<char>* v, const char* str) { append(v, "%s", str); }
}  // namespace

std::unique_ptr<char[]> CreatePoseJSON(const posenet::Output& output,
                                       float threshold) {
    std::vector<char> s;
    s.reserve(2048);

    append(&s, "[");
    for (int i = 0; i < output.num_poses; ++i) {
        if (output.poses[i].score < threshold) continue;

        append(&s, "{\n");
        append(&s, "  \"score\": %g,\n", output.poses[i].score);
        append(&s, "  \"keypoints\": [\n");
        for (int j = 0; j < posenet::kKeypoints; ++j) {
            const auto& kp = output.poses[i].keypoints[j];
            append(&s, "    [%g, %g, %g]", kp.score, kp.x, kp.y);
            append(&s, j != posenet::kKeypoints - 1 ? ",\n" : "\n");
        }
        append(&s, "  ]\n");
        append(&s, "}");
        append(&s, i != output.num_poses - 1 ? ",\n" : "");
    }
    append(&s, "]");

    s.push_back(0);
    auto res = std::make_unique<char[]>(s.size());
    std::memcpy(res.get(), s.data(), s.size());
    return res;
}

}  // namespace oobe
}  // namespace valiant
