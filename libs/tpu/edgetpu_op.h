#ifndef _LIBS_TPU_EDGETPU_OP_H_
#define _LIBS_TPU_EDGETPU_OP_H_

struct TfLiteRegistration;

namespace coral::micro {

static const char kCustomOp[] = "edgetpu-custom-op";
TfLiteRegistration* RegisterCustomOp();

}  // namespace coral::micro

#endif  // _LIBS_TPU_EDGETPU_OP_H_
