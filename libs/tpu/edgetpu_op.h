#ifndef _LIBS_TPU_EDGETPU_OP_H_
#define _LIBS_TPU_EDGETPU_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral::micro {

inline constexpr char kCustomOp[] = "edgetpu-custom-op";

TfLiteRegistration* RegisterCustomOp();

}  // namespace coral::micro

#endif  // _LIBS_TPU_EDGETPU_OP_H_
