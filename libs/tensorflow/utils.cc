#include "libs/tensorflow/utils.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/micro/kernels/kernel_runner.h"
#include "third_party/tensorflow/tensorflow/lite/micro/test_helpers.h"

namespace valiant {
namespace tensorflow {

std::unique_ptr<tflite::MicroInterpreter> MakeEdgeTpuInterpreterInternal(
    const tflite::Model *model,
    EdgeTpuContext *context,
    tflite::MicroOpResolver* resolver,
    tflite::MicroErrorReporter* error_reporter,
    uint8_t *tensor_arena,
    size_t tensor_arena_size)
{
    if (!model || !error_reporter) {
        return nullptr;
    }

    std::unique_ptr<tflite::MicroInterpreter> interpreter(new tflite::MicroInterpreter(model, *resolver, tensor_arena, tensor_arena_size, error_reporter));
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors failed.");
        return nullptr;
    }

    return interpreter;
}

bool ResizeImage(const ImageDims& in_dims, const uint8_t *in,
                 const ImageDims& out_dims, uint8_t* out) {
    if (in_dims == out_dims) {
        memcpy(out, in, ImageSize(in_dims));
        return true;
    }


    // Array with 4 values: batch, rows, columns, depth
    int input_dims_ints[] = {4, 1, in_dims.height, in_dims.width, in_dims.depth};
    TfLiteIntArray* input_dims = tflite::testing::IntArrayFromInts(input_dims_ints);
    // Array with 1 value: the number of dimensions.
    int size_dims_ints[] = {1, 2};
    TfLiteIntArray* size_dims = tflite::testing::IntArrayFromInts(size_dims_ints);
    // Array with 4 values: batch, rows, columns, depth
    int output_dims_ints[] = {4, 1, out_dims.height, out_dims.width, out_dims.depth};
    TfLiteIntArray* output_dims = tflite::testing::IntArrayFromInts(output_dims_ints);

    constexpr int tensors_size = 3;
    int32_t expected_size[] = {out_dims.height, out_dims.width};
    TfLiteTensor tensors[tensors_size] = {
        tflite::testing::CreateQuantizedTensor(in, input_dims, 0, 255),
        tflite::testing::CreateTensor(expected_size, size_dims),
        tflite::testing::CreateQuantizedTensor(out, output_dims, 0, 255)
    };
    tensors[1].allocation_type = kTfLiteMmapRo;

    // Array with 2 values, representing the indices of the input tensors in the tensors array.
    int inputs_ints[] = {2, 0, 1};
    TfLiteIntArray* inputs = tflite::testing::IntArrayFromInts(inputs_ints);
    // Array with 1 value, representing the indices of the output tensors in the tensors array.
    int outputs_ints[] = {1, 2};
    TfLiteIntArray* outputs = tflite::testing::IntArrayFromInts(outputs_ints);

    TfLiteResizeNearestNeighborParams params = {
        false, /* align_corners */
        false /* half_pixel_centers */};

    tflite::micro::KernelRunner runner(
        tflite::ops::micro::Register_RESIZE_NEAREST_NEIGHBOR(),
        tensors, tensors_size,
        inputs, outputs, &params
    );

    if (runner.InitAndPrepare() != kTfLiteOk) {
        printf("failed to init and prepare\r\n");
        return false;
    }

    if (runner.Invoke() != kTfLiteOk) {
        printf("failed to invoke\r\n");
        return false;
    }
    return true;
}

}  // namespace tensorflow
}  // namespace valiant
