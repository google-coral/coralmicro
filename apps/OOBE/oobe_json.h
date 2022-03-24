#ifndef _APPS_OOBE_OOBE_JSON_H_
#define _APPS_OOBE_OOBE_JSON_H_

#include <vector>

#include "libs/posenet/posenet.h"

namespace valiant {
namespace oobe {

std::vector<uint8_t> CreatePoseJSON(const posenet::Output& output,
                                    float threshold);

}  // namespace oobe
}  // namespace valiant

#endif  // _APPS_OOBE_OOBE_JSON_H_
