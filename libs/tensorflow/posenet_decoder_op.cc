/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "posenet_decoder_op.h"

#include <cmath>
#include <numeric>
#include <string>

#include "flatbuffers/flexbuffers.h"
#include "posenet_decoder.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"

namespace coralmicro {
namespace posenet_decoder_op {

using tflite::GetInput;
using tflite::GetOutput;
using tflite::GetTensorData;
using tflite::NumDimensions;
using tflite::NumInputs;
using tflite::NumOutputs;

constexpr int kNumInputs = 4;

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
  void* heatmaps_float_ptr;
  void* shorts_float_ptr;
  void* mids_float_ptr;
  void* longs_float_ptr;

  int zero_point[kNumInputs];
  float scale[kNumInputs];
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

TfLiteStatus PrepTempTensor(TfLiteContext* context, void** temp_tensor_ptr,
                            const TfLiteIntArray* dims) {
  int bytes = sizeof(float);
  for (int i = 0; i < dims->size; ++i) {
    bytes *= dims->data[i];
  }
  *temp_tensor_ptr = context->AllocatePersistentBuffer(context, bytes);
  if (*temp_tensor_ptr) {
    return kTfLiteOk;
  } else {
    return kTfLiteError;
  }
}

void DequantizeTensor(const TfLiteEvalTensor* src, void* dst,
                      const OpData* op_data, const int tensor_type,
                      float extra_scale = 1.0) {
  if (tensor_type >= kNumInputs || tensor_type < 0) {
    assert(false);
  }
  if (src->type == kTfLiteUInt8) {
    const int num_elements = tflite::micro::GetTensorShape(src).FlatSize();
    const uint8_t* src_data = tflite::micro::GetTensorData<uint8_t>(src);
    assert(src_data != nullptr);
    float* dst_data = reinterpret_cast<float*>(dst);
    assert(dst_data != nullptr);
    for (int idx = 0; idx < num_elements; ++idx) {
      dst_data[idx] = (src_data[idx] - op_data->zero_point[tensor_type]) *
                      op_data->scale[tensor_type] * extra_scale;
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

  tflite::MicroContext* micro_context = tflite::GetMicroContext(context);
  TfLiteTensor* heatmaps =
      micro_context->AllocateTempInputTensor(node, kInputTensorHeatmaps);
  TF_LITE_ENSURE(context, heatmaps != nullptr);
  TfLiteTensor* shorts =
      micro_context->AllocateTempInputTensor(node, kInputTensorShortOffsets);
  TF_LITE_ENSURE(context, shorts != nullptr);
  TfLiteTensor* mids =
      micro_context->AllocateTempInputTensor(node, kInputTensorMidOffsets);
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
  op_data->scale[kInputTensorHeatmaps] = heatmaps->params.scale;
  op_data->zero_point[kInputTensorHeatmaps] = heatmaps->params.zero_point;
  TF_LITE_ENSURE_OK(context, PrepTempTensor(context, &op_data->shorts_float_ptr,
                                            shorts->dims));
  op_data->scale[kInputTensorShortOffsets] = shorts->params.scale;
  op_data->zero_point[kInputTensorShortOffsets] = shorts->params.zero_point;
  TF_LITE_ENSURE_OK(
      context, PrepTempTensor(context, &op_data->mids_float_ptr, mids->dims));
  op_data->scale[kInputTensorMidOffsets] = mids->params.scale;
  op_data->zero_point[kInputTensorMidOffsets] = mids->params.zero_point;

  if (compute_masks) {
    TfLiteTensor* longs =
        micro_context->AllocateTempInputTensor(node, kInputTensorLongOffsets);
    TF_LITE_ENSURE(context, longs != nullptr);
    TF_LITE_ENSURE(context, (longs->type == kTfLiteUInt8 ||  //
                             longs->type == kTfLiteFloat32));
    TF_LITE_ENSURE_EQ(context, NumDimensions(longs), 4);
    TF_LITE_ENSURE_EQ(context, longs->dims->data[0], 1);
    TF_LITE_ENSURE_EQ(context, longs->dims->data[3], 2 * kNumKeypoints);

    TF_LITE_ENSURE_OK(
        context,
        PrepTempTensor(context, &op_data->longs_float_ptr, longs->dims));
    op_data->scale[kInputTensorLongOffsets] = longs->params.scale;
    op_data->zero_point[kInputTensorLongOffsets] = longs->params.zero_point;
    micro_context->DeallocateTempTfLiteTensor(longs);
  }

  micro_context->DeallocateTempTfLiteTensor(mids);
  micro_context->DeallocateTempTfLiteTensor(shorts);
  micro_context->DeallocateTempTfLiteTensor(heatmaps);

  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  auto* op_data = reinterpret_cast<OpData*>(node->user_data);

  TF_LITE_ENSURE(context, op_data->stride > 0);
  const TfLiteEvalTensor* heatmaps =
      tflite::micro::GetEvalInput(context, node, kInputTensorHeatmaps);
  TF_LITE_ENSURE(context, heatmaps != nullptr);
  const TfLiteEvalTensor* shorts =
      tflite::micro::GetEvalInput(context, node, kInputTensorShortOffsets);
  TF_LITE_ENSURE(context, shorts != nullptr);
  const TfLiteEvalTensor* mids =
      tflite::micro::GetEvalInput(context, node, kInputTensorMidOffsets);
  TF_LITE_ENSURE(context, mids != nullptr);

  // Dequantize (and rescale) input tensors
  DequantizeTensor(heatmaps, op_data->heatmaps_float_ptr, op_data,
                   kInputTensorHeatmaps);
  DequantizeTensor(shorts, op_data->shorts_float_ptr, op_data,
                   kInputTensorShortOffsets, 1.0 / op_data->stride);
  DequantizeTensor(mids, op_data->mids_float_ptr, op_data,
                   kInputTensorMidOffsets, 1.0 / op_data->stride);

  const float* heatmaps_data =
      reinterpret_cast<float*>(op_data->heatmaps_float_ptr);
  const float* shorts_data =
      reinterpret_cast<float*>(op_data->shorts_float_ptr);
  const float* mids_data = reinterpret_cast<float*>(op_data->mids_float_ptr);

  TfLiteEvalTensor* pose_keypoints =
      tflite::micro::GetEvalOutput(context, node, kOutputTensorPoseKeypoints);
  TF_LITE_ENSURE(context, pose_keypoints != nullptr);
  TfLiteEvalTensor* pose_keypoint_scores = tflite::micro::GetEvalOutput(
      context, node, kOutputTensorPoseKeypointScores);
  TF_LITE_ENSURE(context, pose_keypoint_scores != nullptr);
  TfLiteEvalTensor* pose_scores =
      tflite::micro::GetEvalOutput(context, node, kOutputTensorPoseScores);
  TF_LITE_ENSURE(context, pose_scores != nullptr);
  TfLiteEvalTensor* pose_count =
      tflite::micro::GetEvalOutput(context, node, kOutputTensorPoseCount);
  TF_LITE_ENSURE(context, pose_count != nullptr);

  float* pose_keypoints_data =
      tflite::micro::GetTensorData<float>(pose_keypoints);
  float* pose_keypoint_scores_data =
      tflite::micro::GetTensorData<float>(pose_keypoint_scores);
  float* pose_scores_data = tflite::micro::GetTensorData<float>(pose_scores);
  float* pose_count_data = tflite::micro::GetTensorData<float>(pose_count);

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
    const TfLiteEvalTensor* longs =
        tflite::micro::GetEvalInput(context, node, kInputTensorLongOffsets);
    TF_LITE_ENSURE(context, longs != nullptr);
    DequantizeTensor(longs, op_data->longs_float_ptr, op_data,
                     kInputTensorLongOffsets, 1.0 / op_data->stride);
    const float* longs_data =
        reinterpret_cast<float*>(op_data->longs_float_ptr);
    TfLiteEvalTensor* instance_masks =
        tflite::micro::GetEvalOutput(context, node, kOutputTensorInstanceMasks);
    TF_LITE_ENSURE(context, instance_masks != nullptr);
    float* instance_masks_data =
        tflite::micro::GetTensorData<float>(instance_masks);

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

}  // namespace coralmicro
