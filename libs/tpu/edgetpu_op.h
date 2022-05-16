#ifndef LIBS_TPU_EDGETPU_OP_H_
#define LIBS_TPU_EDGETPU_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral::micro {

inline constexpr char kCustomOp[] = "edgetpu-custom-op";

TfLiteRegistration* RegisterCustomOp();

}  // namespace coral::micro

#endif  // LIBS_TPU_EDGETPU_OP_H_
