#include "apps/OOBE/oobe_json.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace valiant {
namespace oobe {
namespace {
template <typename... T>
void append(std::vector<uint8_t>* v, const char* format, T... args) {
    const int size = std::snprintf(nullptr, 0, format, args...) + 1;
    v->resize(v->size() + size);
    auto* s = reinterpret_cast<char*>(v->data() + v->size() - size);
    std::snprintf(s, size, format, args...);
    v->pop_back();  // remove null terminator
}

void append(std::vector<uint8_t>* v, const char* str) { append(v, "%s", str); }
}  // namespace

std::vector<uint8_t> CreatePoseJSON(const posenet::Output& output,
                                    float threshold) {
    std::vector<uint8_t> s;
    s.reserve(2048);

    int num_appended_poses = 0;
    append(&s, "[");
    for (int i = 0; i < output.num_poses; ++i) {
        if (output.poses[i].score < threshold) continue;

        append(&s, num_appended_poses != 0 ? ",\n" : "");
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
        ++num_appended_poses;
    }
    append(&s, "]");
    return s;
}

}  // namespace oobe
}  // namespace valiant
