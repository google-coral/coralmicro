#include "posenet_decoder_op.h"

#include <cmath>
#include <numeric>
#include <string>

#include "posenet_decoder.h"
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"

namespace coral {
namespace posenet_decoder_op {

using tflite::GetInput;
using tflite::GetOutput;
using tflite::GetTensorData;
using tflite::NumDimensions;
using tflite::NumInputs;
using tflite::NumOutputs;

constexpr int kInputTensorHeatmaps = 0;
constexpr int kInputTensorShortOffsets = 1;
constexpr int kInputTensorMidOffsets = 2;
constexpr int kInputTensorLongOffsets = 3;

constexpr int kOutputTensorPoseKeypoints = 0;
constexpr int kOutputTensorPoseKeypointScores = 1;
constexpr int kOutputTensorPoseScores = 2;
constexpr int kOutputTensorPoseCount = 3;
constexpr int kOutputTensorInstanceMasks = 4;

struct OpData {
  // Decoder parameters
  int max_detections;
  float score_threshold;
  int stride;
  float nms_radius;

  // Temporary tensors (e.g for dequantized values)
  void *heatmaps_float_ptr;
  void *shorts_float_ptr;
  void *mids_float_ptr;
  void *longs_float_ptr;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  auto* op_data = new OpData;
  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();

  op_data->max_detections = m["max_detections"].AsInt32();
  op_data->score_threshold = m["score_threshold"].AsFloat();
  op_data->stride = m["stride"].AsInt32();
  op_data->nms_radius = m["nms_radius"].AsFloat();

  return op_data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus PrepTempTensor(TfLiteContext *context, void **temp_tensor_ptr,
                            const TfLiteIntArray *dims) {
  int bytes = sizeof(float);
  for (int i = 0; i < dims->size; ++i) {
    bytes *= dims->data[i];
  }
  return context->AllocatePersistentBuffer(context, bytes, temp_tensor_ptr);
}

TfLiteStatus PrepOutputTensor(TfLiteContext* context,
                              TfLiteTensor* output_tensor,
                              std::initializer_list<int> dims) {
  output_tensor->type = kTfLiteFloat32;
  TfLiteIntArray* size = TfLiteIntArrayCreate(dims.size());
  std::copy(std::begin(dims), std::end(dims), size->data);
  output_tensor->dims = size;
  output_tensor->bytes = sizeof(float);
  for (int i = 0; i < size->size; ++i) {
    output_tensor->bytes *= size->data[i];
  }
  TfLiteStatus ret = context->AllocatePersistentBuffer(context, output_tensor->bytes, (void**)&output_tensor->data);
  return ret;
}

void DequantizeTensor(const TfLiteTensor* src, void* dst,
                      float extra_scale = 1.0) {
  if (src->type == kTfLiteUInt8) {
    const int num_elements = src->bytes;
    const float quant_zero_point = static_cast<float>(src->params.zero_point);
    const float quant_scale = src->params.scale * extra_scale;
    const uint8_t* src_data = GetTensorData<uint8_t>(src);
    assert(src_data != nullptr);
    float* dst_data = reinterpret_cast<float*>(dst);
    assert(dst_data != nullptr);
    for (int idx = 0; idx < num_elements; ++idx) {
      dst_data[idx] = (src_data[idx] - quant_zero_point) * quant_scale;
    }
  } else {
    assert(false);
  }
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  auto* op_data = reinterpret_cast<OpData*>(node->user_data);
  TF_LITE_ENSURE(context, ((NumInputs(node) == 3 && NumOutputs(node) == 4) ||
                           (NumInputs(node) == 4 && NumOutputs(node) == 5)));
  bool compute_masks = false;
  if (NumInputs(node) == 4 && NumOutputs(node) == 5) {
    compute_masks = true;
  }

  const TfLiteTensor* heatmaps = GetInput(context, node, kInputTensorHeatmaps);
  TF_LITE_ENSURE(context, heatmaps != nullptr);
  const TfLiteTensor* shorts =
      GetInput(context, node, kInputTensorShortOffsets);
  TF_LITE_ENSURE(context, shorts != nullptr);
  const TfLiteTensor* mids = GetInput(context, node, kInputTensorMidOffsets);
  TF_LITE_ENSURE(context, mids != nullptr);

  TF_LITE_ENSURE(context, (heatmaps->type == kTfLiteUInt8 ||  //
                           heatmaps->type == kTfLiteFloat32));
  TF_LITE_ENSURE(context, (shorts->type == kTfLiteUInt8 ||  //
                           shorts->type == kTfLiteFloat32));
  TF_LITE_ENSURE(context, (mids->type == kTfLiteUInt8 ||  //
                           mids->type == kTfLiteFloat32));
  TF_LITE_ENSURE_EQ(context, NumDimensions(heatmaps), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(shorts), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(mids), 4);
  TF_LITE_ENSURE_EQ(context, heatmaps->dims->data[0], 1);
  TF_LITE_ENSURE_EQ(context, shorts->dims->data[0], 1);
  TF_LITE_ENSURE_EQ(context, mids->dims->data[0], 1);
  TF_LITE_ENSURE_EQ(context, heatmaps->dims->data[3], kNumKeypoints);
  TF_LITE_ENSURE_EQ(context, shorts->dims->data[3], 2 * kNumKeypoints);
  TF_LITE_ENSURE_EQ(context, mids->dims->data[3], 2 * 2 * kNumEdges);

  TF_LITE_ENSURE_OK(
      context,
      PrepTempTensor(context, &op_data->heatmaps_float_ptr, heatmaps->dims));
  TF_LITE_ENSURE_OK(
      context,
      PrepTempTensor(context, &op_data->shorts_float_ptr, shorts->dims));
  TF_LITE_ENSURE_OK(
      context, PrepTempTensor(context, &op_data->mids_float_ptr, mids->dims));

  // Output tensor 0 to be max_detections*kNumKeypoints*2
  // The last dimension has the x and y coordinates of each keypoint.
  TF_LITE_ENSURE_OK(
      context,
      PrepOutputTensor(context,
                       GetOutput(context, node, kOutputTensorPoseKeypoints),
                       {1, op_data->max_detections, kNumKeypoints, 2}));

  // Output tensor 1 to be size max_detections*kNumKeypoints and contain
  // keypoints scores in the range [0,1].
  TF_LITE_ENSURE_OK(
      context,
      PrepOutputTensor(
          context, GetOutput(context, node, kOutputTensorPoseKeypointScores),
          {1, op_data->max_detections, kNumKeypoints}));

  // Output tensor 2 to be size max_detections and contain
  // pose scores in the range [0,1].
  TF_LITE_ENSURE_OK(
      context, PrepOutputTensor(
                   context, GetOutput(context, node, kOutputTensorPoseScores),
                   {1, op_data->max_detections}));

  // Output Tensor 3 is an int32 scalar, the number of detected poses.
  // Currently only float output tensors are supported so save this as a float.
  TF_LITE_ENSURE_OK(
      context,
      PrepOutputTensor(context,
                       GetOutput(context, node, kOutputTensorPoseCount), {1}));

  if (compute_masks) {
    const TfLiteTensor* longs =
        GetInput(context, node, kInputTensorLongOffsets);
    TF_LITE_ENSURE(context, longs != nullptr);
    TF_LITE_ENSURE(context, (longs->type == kTfLiteUInt8 ||  //
                             longs->type == kTfLiteFloat32));
    TF_LITE_ENSURE_EQ(context, NumDimensions(longs), 4);
    TF_LITE_ENSURE_EQ(context, longs->dims->data[0], 1);
    TF_LITE_ENSURE_EQ(context, longs->dims->data[3], 2 * kNumKeypoints);

    TF_LITE_ENSURE_OK(
        context,
        PrepTempTensor(context, &op_data->longs_float_ptr, longs->dims));

    // Output tensor 4 to be max_detections*33*33 (where long_offsets is 33x33)
    // and contain max_detections of 33x33 person instance segmentation masks.
    TF_LITE_ENSURE_OK(
        context,
        PrepOutputTensor(context,
                         GetOutput(context, node, kOutputTensorInstanceMasks),
                         {1, op_data->max_detections, 33, 33}));
  }

  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  auto* op_data = reinterpret_cast<OpData*>(node->user_data);

  TF_LITE_ENSURE(context, op_data->stride > 0);
  const TfLiteTensor* heatmaps = GetInput(context, node, kInputTensorHeatmaps);
  TF_LITE_ENSURE(context, heatmaps != nullptr);
  const TfLiteTensor* shorts =
      GetInput(context, node, kInputTensorShortOffsets);
  TF_LITE_ENSURE(context, shorts != nullptr);
  const TfLiteTensor* mids = GetInput(context, node, kInputTensorMidOffsets);
  TF_LITE_ENSURE(context, mids != nullptr);

  // Dequantize (and rescale) input tensors
  DequantizeTensor(heatmaps, op_data->heatmaps_float_ptr);
  DequantizeTensor(shorts, op_data->shorts_float_ptr, 1.0 / op_data->stride);
  DequantizeTensor(mids, op_data->mids_float_ptr, 1.0 / op_data->stride);

  const float* heatmaps_data = reinterpret_cast<float*>(op_data->heatmaps_float_ptr);
  const float* shorts_data = reinterpret_cast<float*>(op_data->shorts_float_ptr);
  const float* mids_data = reinterpret_cast<float*>(op_data->mids_float_ptr);

  TfLiteTensor* pose_keypoints =
      GetOutput(context, node, kOutputTensorPoseKeypoints);
  TF_LITE_ENSURE(context, pose_keypoints != nullptr);
  TfLiteTensor* pose_keypoint_scores =
      GetOutput(context, node, kOutputTensorPoseKeypointScores);
  TF_LITE_ENSURE(context, pose_keypoint_scores != nullptr);
  TfLiteTensor* pose_scores = GetOutput(context, node, kOutputTensorPoseScores);
  TF_LITE_ENSURE(context, pose_scores != nullptr);
  TfLiteTensor* pose_count = GetOutput(context, node, kOutputTensorPoseCount);
  TF_LITE_ENSURE(context, pose_count != nullptr);

  float* pose_keypoints_data = GetTensorData<float>(pose_keypoints);
  float* pose_keypoint_scores_data = GetTensorData<float>(pose_keypoint_scores);
  float* pose_scores_data = GetTensorData<float>(pose_scores);
  float* pose_count_data = GetTensorData<float>(pose_count);

  const float nms_radius = op_data->nms_radius / op_data->stride;
  pose_count_data[0] = DecodeAllPoses(
      heatmaps_data, shorts_data, mids_data,
      /*height = */ heatmaps->dims->data[1],
      /*width = */ heatmaps->dims->data[2], op_data->max_detections,
      op_data->score_threshold,
      /*mid_short_offset_refinement_steps = */ 5, nms_radius, op_data->stride,
      reinterpret_cast<PoseKeypoints*>(pose_keypoints_data),
      reinterpret_cast<PoseKeypointScores*>(pose_keypoint_scores_data),
      pose_scores_data);

  if (NumInputs(node) == 4) {
    const TfLiteTensor* longs =
        GetInput(context, node, kInputTensorLongOffsets);
    TF_LITE_ENSURE(context, longs != nullptr);
    DequantizeTensor(longs, op_data->longs_float_ptr, 1.0 / op_data->stride);
    const float* longs_data = reinterpret_cast<float*>(op_data->longs_float_ptr);
    TfLiteTensor* instance_masks =
        GetOutput(context, node, kOutputTensorInstanceMasks);
    TF_LITE_ENSURE(context, instance_masks != nullptr);
    float* instance_masks_data = GetTensorData<float>(instance_masks);

    DecodeInstanceMasks(longs_data, /*height = */ longs->dims->data[1],
                        /*width = */ longs->dims->data[2],
                        reinterpret_cast<PoseKeypoints*>(pose_keypoints_data),
                        /*num_poses = */ pose_count_data[0],
                        /*refinement_steps = */ 2, op_data->stride,
                        instance_masks_data);
  }

  return kTfLiteOk;
}

}  // namespace posenet_decoder_op

TfLiteRegistration* RegisterPosenetDecoderOp() {
  static TfLiteRegistration r = {
      posenet_decoder_op::Init, posenet_decoder_op::Free,
      posenet_decoder_op::Prepare, posenet_decoder_op::Eval};
  return &r;
}

}  // namespace coral
