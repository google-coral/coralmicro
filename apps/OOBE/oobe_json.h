#ifndef _APPS_OOBE_OOBE_JSON_H_
#define _APPS_OOBE_OOBE_JSON_H_

#include "libs/posenet/posenet.h"
#include <memory>

namespace valiant {
namespace oobe {

std::unique_ptr<char> CreatePoseJSON(unsigned int id, unsigned int now, const posenet::Output& output, int pose);
std::unique_ptr<char> CreateFrameJSON(unsigned int id, unsigned int now, unsigned int good_poses_count, const char *url);

}  // namespace valiant
}  // namespace oobe

#endif  // _APPS_OOBE_OOBE_JSON_H_