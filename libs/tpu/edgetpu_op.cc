#include "libs/tpu/edgetpu_op.h"

#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coral::micro {
namespace {
void* CustomOpInit(TfLiteContext* context, const char* buffer, size_t length) {
    return EdgeTpuManager::GetSingleton()->RegisterPackage(buffer, length);
}

void CustomOpFree(TfLiteContext* context, void* buffer) {}

TfLiteStatus CustomOpPrepare(TfLiteContext* context, TfLiteNode* node) {
    if (node->user_data == nullptr) return kTfLiteError;
    return kTfLiteOk;
}

TfLiteStatus CustomOpInvoke(TfLiteContext* context, TfLiteNode* node) {
    EdgeTpuPackage* package = static_cast<EdgeTpuPackage*>(node->user_data);
    return EdgeTpuManager::GetSingleton()->Invoke(package, context, node);
}
}  // namespace

TfLiteRegistration* RegisterCustomOp() {
    static TfLiteRegistration registration = {
        CustomOpInit,
        CustomOpFree,
        CustomOpPrepare,
        CustomOpInvoke,
    };
    return &registration;
}
}  // namespace coral::micro
