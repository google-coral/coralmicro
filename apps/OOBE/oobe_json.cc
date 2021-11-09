#include "apps/OOBE/oobe_json.h"
#include "third_party/mjson/src/mjson.h"

namespace valiant {
namespace oobe {

static const char * pose_json_prefix =
        "[";

static const char * pose_json_pose =
        "{ \
        \"score\":%g, \
        \"keypoints\":[ \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g], \
            [%g,%g,%g]  \
        ] \
        }";

static const char *pose_json_suffix =
    "]";

std::unique_ptr<char[]> CreatePoseJSON(const posenet::Output& output, float threshold) {
    std::string fmt_str;
    {
        std::unique_ptr<char> json_buf(mjson_aprintf(oobe::pose_json_prefix));
        fmt_str.append(json_buf.get());
    }
    bool first = true;
    for (int i = 0; i < valiant::posenet::kPoses; ++i) {
        if (output.poses[i].score < threshold) {
            continue;
        }
        if (!first) {
            fmt_str.append(",");
        }
        {
            std::unique_ptr<char> json_buf(mjson_aprintf(oobe::pose_json_pose,
            output.poses[i].score,
            output.poses[i].keypoints[0].score, output.poses[i].keypoints[0].x, output.poses[i].keypoints[0].y,
            output.poses[i].keypoints[1].score, output.poses[i].keypoints[1].x, output.poses[i].keypoints[1].y,
            output.poses[i].keypoints[2].score, output.poses[i].keypoints[2].x, output.poses[i].keypoints[2].y,
            output.poses[i].keypoints[3].score, output.poses[i].keypoints[3].x, output.poses[i].keypoints[3].y,
            output.poses[i].keypoints[4].score, output.poses[i].keypoints[4].x, output.poses[i].keypoints[4].y,
            output.poses[i].keypoints[5].score, output.poses[i].keypoints[5].x, output.poses[i].keypoints[5].y,
            output.poses[i].keypoints[6].score, output.poses[i].keypoints[6].x, output.poses[i].keypoints[6].y,
            output.poses[i].keypoints[7].score, output.poses[i].keypoints[7].x, output.poses[i].keypoints[7].y,
            output.poses[i].keypoints[8].score, output.poses[i].keypoints[8].x, output.poses[i].keypoints[8].y,
            output.poses[i].keypoints[9].score, output.poses[i].keypoints[9].x, output.poses[i].keypoints[9].y,
            output.poses[i].keypoints[10].score, output.poses[i].keypoints[10].x, output.poses[i].keypoints[10].y,
            output.poses[i].keypoints[11].score, output.poses[i].keypoints[11].x, output.poses[i].keypoints[11].y,
            output.poses[i].keypoints[12].score, output.poses[i].keypoints[12].x, output.poses[i].keypoints[12].y,
            output.poses[i].keypoints[13].score, output.poses[i].keypoints[13].x, output.poses[i].keypoints[13].y,
            output.poses[i].keypoints[14].score, output.poses[i].keypoints[14].x, output.poses[i].keypoints[14].y,
            output.poses[i].keypoints[15].score, output.poses[i].keypoints[15].x, output.poses[i].keypoints[15].y,
            output.poses[i].keypoints[16].score, output.poses[i].keypoints[16].x, output.poses[i].keypoints[16].y
            ));
            fmt_str.append(json_buf.get());
        }
        first = false;
    }
    fmt_str.append(pose_json_suffix);
    auto s = std::make_unique<char[]>(fmt_str.size() + 1);
    memcpy(s.get(), fmt_str.data(), fmt_str.size());
    s[fmt_str.size()] = 0;
    return s;
}

}  // namespace oobe
}  // namespace valiant
