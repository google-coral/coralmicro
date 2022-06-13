#ifndef LIBS_POSENET_POSENET_DECODER_OP_H_
#define LIBS_POSENET_POSENET_DECODER_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral {

// Posenet custom op name. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
inline constexpr char kPosenetDecoderOp[] = "PosenetDecoderOp";

// Returns pointer to an instance of `tflite::TfLiteRegistration` to handle
// Posenet custom ops. Pass this to
// `tflite::MicroMutableOpResolver::AddCustom()`.
TfLiteRegistration* RegisterPosenetDecoderOp();

}  // namespace coral

#endif  // LIBS_POSENET_POSENET_DECODER_OP_H_
