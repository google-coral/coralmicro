#ifndef _APPS_OOBE_OOBE_JSON_H_
#define _APPS_OOBE_OOBE_JSON_H_

#include "libs/posenet/posenet.h"
#include <memory>

namespace valiant {
namespace oobe {

std::unique_ptr<char[]> CreatePoseJSON(const posenet::Output& output, float threshold);

}  // namespace valiant
}  // namespace oobe

#endif  // _APPS_OOBE_OOBE_JSON_H_
