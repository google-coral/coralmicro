#ifndef LIBS_TPU_EDGETPU_OP_H_
#define LIBS_TPU_EDGETPU_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral::micro {

// Edge TPU custom op. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
inline constexpr char kCustomOp[] = "edgetpu-custom-op";

// Returns pointer to an instance of `tflite::TfLiteRegistration` to handle
// Edge TPU custom ops. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
TfLiteRegistration* RegisterCustomOp();

}  // namespace coral::micro

#endif  // LIBS_TPU_EDGETPU_OP_H_
