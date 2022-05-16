#ifndef LIBS_POSENET_POSENET_DECODER_OP_H_
#define LIBS_POSENET_POSENET_DECODER_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral {

inline constexpr char kPosenetDecoderOp[] = "PosenetDecoderOp";

TfLiteRegistration* RegisterPosenetDecoderOp();

}  // namespace coral

#endif  // LIBS_POSENET_POSENET_DECODER_OP_H_
