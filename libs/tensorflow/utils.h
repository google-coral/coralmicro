#ifndef _LIBS_TENSORFLOW_UTILS_H_
#define _LIBS_TENSORFLOW_UTILS_H_

#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace valiant {
namespace tensorflow {

struct ImageDims {
  int height;
  int width;
  int depth;
};

inline bool operator==(const ImageDims& a, const ImageDims& b) {
  return a.height == b.height && a.width == b.width && a.depth == b.depth;
}
inline int ImageSize(const ImageDims& dims) {
  return dims.height * dims.width * dims.depth;
}

bool ResizeImage(const ImageDims& in_dims, const uint8_t *uin,
                 const ImageDims& out_dims, uint8_t* uout);

inline int TensorSize(TfLiteTensor* tensor) {
    int size = 1;
    for (int i = 0; i < tensor->dims->size; ++i) {
        size *= tensor->dims->data[i];
    }
    return size;
}

template <typename I, typename O>
void Dequantize(int count, I* tensor_data, O* dequant_data, float scale, float zero_point) {
    for (int i = 0; i < count; ++i) {
        dequant_data[i] = scale * (tensor_data[i] - zero_point);
    }
}

template <typename T>
std::vector<T> DequantizeTensor(TfLiteTensor* tensor) {
  const auto scale = tensor->params.scale;
  const auto zero_point = tensor->params.zero_point;
  std::vector<T> result(TensorSize(tensor));

  if (tensor->type == kTfLiteUInt8) {
    Dequantize(result.size(), tflite::GetTensorData<uint8_t>(tensor), result.data(), scale, zero_point);
  } else if (tensor->type == kTfLiteInt8) {
    Dequantize(result.size(), tflite::GetTensorData<int8_t>(tensor), result.data(), scale, zero_point);
  } else {
      assert(false);
  }

  return result;
}

std::unique_ptr<tflite::MicroInterpreter> MakeEdgeTpuInterpreterInternal(
    const tflite::Model *model,
    EdgeTpuContext *context,
    tflite::MicroOpResolver* resolver,
    tflite::MicroErrorReporter* error_reporter,
    uint8_t *tensor_arena,
    size_t tensor_arena_size
    );

template <unsigned int tOpCount>
std::unique_ptr<tflite::MicroInterpreter> MakeEdgeTpuInterpreter(
    const tflite::Model *model,
    EdgeTpuContext *context,
    tflite::MicroMutableOpResolver<tOpCount>* resolver,
    tflite::MicroErrorReporter* error_reporter,
    uint8_t *tensor_arena,
    size_t tensor_arena_size
    ) {
        if (!resolver) {
            return nullptr;
        }
        if (!resolver->FindOp(kCustomOp)) {
            resolver->AddCustom(kCustomOp, RegisterCustomOp());
        }
        return MakeEdgeTpuInterpreterInternal(model, context, resolver, error_reporter, tensor_arena, tensor_arena_size);
    }

}  // namespace tensorflow
}  // namespace valiant

#endif  // _LIBS_TENSORFLOW_UTILS_H_
