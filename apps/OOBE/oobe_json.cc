#include "apps/OOBE/oobe_json.h"
#include "third_party/mjson/src/mjson.h"

namespace valiant {
namespace oobe {

static const char * pose_json =
    "{\"jsonrpc\":\"2.0\", \"method\": \"pose\", \
    \"id\":%u, \
    \"params\":{ \
        \"now\":%u, \
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
    } \
    }";

std::unique_ptr<char> CreatePoseJSON(unsigned int id, unsigned int now, const posenet::Output& output, int pose) {
    std::unique_ptr<char> s(
        mjson_aprintf(
            oobe::pose_json, 0,
            now,
            output.poses[pose].score,
            output.poses[pose].keypoints[0].score, output.poses[pose].keypoints[0].x, output.poses[pose].keypoints[0].y,
            output.poses[pose].keypoints[1].score, output.poses[pose].keypoints[1].x, output.poses[pose].keypoints[1].y,
            output.poses[pose].keypoints[2].score, output.poses[pose].keypoints[2].x, output.poses[pose].keypoints[2].y,
            output.poses[pose].keypoints[3].score, output.poses[pose].keypoints[3].x, output.poses[pose].keypoints[3].y,
            output.poses[pose].keypoints[4].score, output.poses[pose].keypoints[4].x, output.poses[pose].keypoints[4].y,
            output.poses[pose].keypoints[5].score, output.poses[pose].keypoints[5].x, output.poses[pose].keypoints[5].y,
            output.poses[pose].keypoints[6].score, output.poses[pose].keypoints[6].x, output.poses[pose].keypoints[6].y,
            output.poses[pose].keypoints[7].score, output.poses[pose].keypoints[7].x, output.poses[pose].keypoints[7].y,
            output.poses[pose].keypoints[8].score, output.poses[pose].keypoints[8].x, output.poses[pose].keypoints[8].y,
            output.poses[pose].keypoints[9].score, output.poses[pose].keypoints[9].x, output.poses[pose].keypoints[9].y,
            output.poses[pose].keypoints[10].score, output.poses[pose].keypoints[10].x, output.poses[pose].keypoints[10].y,
            output.poses[pose].keypoints[11].score, output.poses[pose].keypoints[11].x, output.poses[pose].keypoints[11].y,
            output.poses[pose].keypoints[12].score, output.poses[pose].keypoints[12].x, output.poses[pose].keypoints[12].y,
            output.poses[pose].keypoints[13].score, output.poses[pose].keypoints[13].x, output.poses[pose].keypoints[13].y,
            output.poses[pose].keypoints[14].score, output.poses[pose].keypoints[14].x, output.poses[pose].keypoints[14].y,
            output.poses[pose].keypoints[15].score, output.poses[pose].keypoints[15].x, output.poses[pose].keypoints[15].y,
            output.poses[pose].keypoints[16].score, output.poses[pose].keypoints[16].x, output.poses[pose].keypoints[16].y
        )
    );
    return s;
}

const char *frame_json =
    "{\"jsonrpc\":\"2.0\", \"id\":%d, \"method\":\"frame\", \"params\":{ \
        \"now\":%d, \
        \"poses\":%d, \
        \"url\":%Q \
    }}";

std::unique_ptr<char> CreateFrameJSON(unsigned int id, unsigned int now, unsigned int good_poses_count, const char *url) {
    std::unique_ptr<char> s(
        mjson_aprintf(frame_json, id, now, good_poses_count, url)
    );
    return s;
}

}  // namespace oobe
}  // namespace valiant
