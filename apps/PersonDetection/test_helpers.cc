/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "apps/PersonDetection/test_helpers.h"

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>

#include "flatbuffers/flatbuffers.h"  // from @flatbuffers
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace tflite {
namespace testing {

flatbuffers::FlatBufferBuilder* BuilderInstance() {
  static char inst_memory[sizeof(flatbuffers::FlatBufferBuilder)];
  static flatbuffers::FlatBufferBuilder* inst =
      new (inst_memory) flatbuffers::FlatBufferBuilder(
          StackAllocator::kStackAllocatorSize, &StackAllocator::instance());
  return inst;
}


ModelBuilder::Operator ModelBuilder::RegisterOp(BuiltinOperator op,
                                                const char* custom_code,
                                                int32_t version) {
  TFLITE_DCHECK(next_operator_code_id_ <= kMaxOperatorCodes);
  operator_codes_[next_operator_code_id_] =
      tflite::CreateOperatorCodeDirect(*builder_, op, custom_code, version);
  next_operator_code_id_++;
  return next_operator_code_id_ - 1;
}

ModelBuilder::Node ModelBuilder::AddNode(
    ModelBuilder::Operator op,
    std::initializer_list<ModelBuilder::Tensor> inputs,
    std::initializer_list<ModelBuilder::Tensor> outputs) {
  TFLITE_DCHECK(next_operator_id_ <= kMaxOperators);
  operators_[next_operator_id_] = tflite::CreateOperator(
      *builder_, op, builder_->CreateVector(inputs.begin(), inputs.size()),
      builder_->CreateVector(outputs.begin(), outputs.size()),
      BuiltinOptions_NONE);
  next_operator_id_++;
  return next_operator_id_ - 1;
}

void ModelBuilder::AddMetadata(const char* description_string,
                               const int32_t* metadata_buffer_data,
                               size_t num_elements) {
  metadata_[ModelBuilder::nbr_of_metadata_buffers_] =
      CreateMetadata(*builder_, builder_->CreateString(description_string),
                     1 + ModelBuilder::nbr_of_metadata_buffers_);

  metadata_buffers_[nbr_of_metadata_buffers_] = tflite::CreateBuffer(
      *builder_, builder_->CreateVector((uint8_t*)metadata_buffer_data,
                                        sizeof(uint32_t) * num_elements));

  ModelBuilder::nbr_of_metadata_buffers_++;
}

const Model* ModelBuilder::BuildModel(
    std::initializer_list<ModelBuilder::Tensor> inputs,
    std::initializer_list<ModelBuilder::Tensor> outputs) {
  // Model schema requires an empty buffer at idx 0.
  size_t buffer_size = 1 + ModelBuilder::nbr_of_metadata_buffers_;
  flatbuffers::Offset<Buffer> buffers[kMaxMetadataBuffers];
  buffers[0] = tflite::CreateBuffer(*builder_);

  // Place the metadata buffers first in the buffer since the indices for them
  // have already been set in AddMetadata()
  for (int i = 1; i < ModelBuilder::nbr_of_metadata_buffers_ + 1; ++i) {
    buffers[i] = metadata_buffers_[i - 1];
  }

  // TFLM only supports single subgraph.
  constexpr size_t subgraphs_size = 1;
  const flatbuffers::Offset<SubGraph> subgraphs[subgraphs_size] = {
      tflite::CreateSubGraph(
          *builder_, builder_->CreateVector(tensors_, next_tensor_id_),
          builder_->CreateVector(inputs.begin(), inputs.size()),
          builder_->CreateVector(outputs.begin(), outputs.size()),
          builder_->CreateVector(operators_, next_operator_id_),
          builder_->CreateString("test_subgraph"))};

  flatbuffers::Offset<Model> model_offset;
  if (ModelBuilder::nbr_of_metadata_buffers_ > 0) {
    model_offset = tflite::CreateModel(
        *builder_, 0,
        builder_->CreateVector(operator_codes_, next_operator_code_id_),
        builder_->CreateVector(subgraphs, subgraphs_size),
        builder_->CreateString("teset_model"),
        builder_->CreateVector(buffers, buffer_size), 0,
        builder_->CreateVector(metadata_,
                               ModelBuilder::nbr_of_metadata_buffers_));
  } else {
    model_offset = tflite::CreateModel(
        *builder_, 0,
        builder_->CreateVector(operator_codes_, next_operator_code_id_),
        builder_->CreateVector(subgraphs, subgraphs_size),
        builder_->CreateString("teset_model"),
        builder_->CreateVector(buffers, buffer_size));
  }

  tflite::FinishModelBuffer(*builder_, model_offset);
  void* model_pointer = builder_->GetBufferPointer();
  const Model* model = flatbuffers::GetRoot<Model>(model_pointer);
  return model;
}

ModelBuilder::Tensor ModelBuilder::AddTensorImpl(
    TensorType type, bool is_variable, std::initializer_list<int32_t> shape) {
  TFLITE_DCHECK(next_tensor_id_ <= kMaxTensors);
  tensors_[next_tensor_id_] = tflite::CreateTensor(
      *builder_, builder_->CreateVector(shape.begin(), shape.size()), type,
      /* buffer */ 0, /* name */ 0, /* quantization */ 0,
      /* is_variable */ is_variable,
      /* sparsity */ 0);
  next_tensor_id_++;
  return next_tensor_id_ - 1;
}

}  // namespace testing
}  // namespace tflite
