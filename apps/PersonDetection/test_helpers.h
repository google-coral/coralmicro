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

#ifndef TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_
#define TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_

// Useful functions for writing tests.

#include <cstdint>

#include "flatbuffers/flatbuffers.h"  // from @flatbuffers
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace tflite {
namespace testing {

class StackAllocator : public flatbuffers::Allocator {
 public:
  StackAllocator() : data_(data_backing_), data_size_(0) {}

  uint8_t* allocate(size_t size) override {
    TFLITE_DCHECK((data_size_ + size) <= kStackAllocatorSize);
    uint8_t* result = data_;
    data_ += size;
    data_size_ += size;
    return result;
  }

  void deallocate(uint8_t* p, size_t) override {}

  static StackAllocator& instance() {
    // Avoid using true dynamic memory allocation to be portable to bare metal.
    static char inst_memory[sizeof(StackAllocator)];
    static StackAllocator* inst = new (inst_memory) StackAllocator;
    return *inst;
  }

  static constexpr size_t kStackAllocatorSize = 8192;

 private:
  uint8_t data_backing_[kStackAllocatorSize];
  uint8_t* data_;
  int data_size_;
};

flatbuffers::FlatBufferBuilder* BuilderInstance();

// A wrapper around FlatBuffer API to help build model easily.
class ModelBuilder {
 public:
  typedef int32_t Tensor;
  typedef int Operator;
  typedef int Node;

  // `builder` needs to be available until BuildModel is called.
  explicit ModelBuilder(flatbuffers::FlatBufferBuilder* builder)
      : builder_(builder) {}

  // Registers an operator that will be used in the model.
  Operator RegisterOp(BuiltinOperator op, const char* custom_code,
                      int32_t version);

  // Adds a tensor to the model.
  Tensor AddTensor(TensorType type, std::initializer_list<int32_t> shape) {
    return AddTensorImpl(type, /* is_variable */ false, shape);
  }

  // Adds a variable tensor to the model.
  Tensor AddVariableTensor(TensorType type,
                           std::initializer_list<int32_t> shape) {
    return AddTensorImpl(type, /* is_variable */ true, shape);
  }

  // Adds a node to the model with given input and output Tensors.
  Node AddNode(Operator op, std::initializer_list<Tensor> inputs,
               std::initializer_list<Tensor> outputs);

  void AddMetadata(const char* description_string,
                   const int32_t* metadata_buffer_data, size_t num_elements);

  // Constructs the flatbuffer model using `builder_` and return a pointer to
  // it. The returned model has the same lifetime as `builder_`.
  const Model* BuildModel(std::initializer_list<Tensor> inputs,
                          std::initializer_list<Tensor> outputs);

 private:
  // Adds a tensor to the model.
  Tensor AddTensorImpl(TensorType type, bool is_variable,
                       std::initializer_list<int32_t> shape);

  flatbuffers::FlatBufferBuilder* builder_;

  static constexpr int kMaxOperatorCodes = 10;
  flatbuffers::Offset<tflite::OperatorCode> operator_codes_[kMaxOperatorCodes];
  int next_operator_code_id_ = 0;

  static constexpr int kMaxOperators = 50;
  flatbuffers::Offset<tflite::Operator> operators_[kMaxOperators];
  int next_operator_id_ = 0;

  static constexpr int kMaxTensors = 50;
  flatbuffers::Offset<tflite::Tensor> tensors_[kMaxTensors];

  static constexpr int kMaxMetadataBuffers = 10;

  static constexpr int kMaxMetadatas = 10;
  flatbuffers::Offset<Metadata> metadata_[kMaxMetadatas];

  flatbuffers::Offset<Buffer> metadata_buffers_[kMaxMetadataBuffers];

  int nbr_of_metadata_buffers_ = 0;

  int next_tensor_id_ = 0;
};

}  // namespace testing
}  // namespace tflite

#endif  // TENSORFLOW_LITE_MICRO_TEST_HELPERS_H_
