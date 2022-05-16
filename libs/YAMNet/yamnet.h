#ifndef LIBS_YAMNET_YAMNET_H_
#define LIBS_YAMNET_YAMNET_H_

#include <vector>
#include "libs/tensorflow/classification.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coral::micro {
namespace yamnet {
    bool setup();
    bool loop(std::shared_ptr<std::vector<tensorflow::Class>> output);
    bool loop(std::shared_ptr<std::vector<tensorflow::Class>> output, bool print);
    std::shared_ptr<TfLiteTensor> input();
} // namespace yamnet
} // namespace coral::micro

#endif  // LIBS_YAMNET_YAMNET_H_
