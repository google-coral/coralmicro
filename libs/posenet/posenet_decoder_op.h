#ifndef THIRD_PARTY_CORAL_LIBCORAL_CORAL_POSENET_POSENET_DECODER_OP_H_
#define THIRD_PARTY_CORAL_LIBCORAL_CORAL_POSENET_POSENET_DECODER_OP_H_

#include "tensorflow/lite/c/common.h"

namespace coral {

inline constexpr char kPosenetDecoderOp[] = "PosenetDecoderOp";

TfLiteRegistration* RegisterPosenetDecoderOp();

}  // namespace coral

#endif  // THIRD_PARTY_CORAL_LIBCORAL_CORAL_POSENET_POSENET_DECODER_OP_H_
