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

#include "libs/tpu/edgetpu_executable.h"

#include "tensorflow/lite/micro/kernels/kernel_util.h"

namespace {
int TensorDataTypeSize(platforms::darwinn::DataType data_type) {
  switch (data_type) {
    case platforms::darwinn::DataType_FIXED_POINT8:
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT8:
      return 1;
    case platforms::darwinn::DataType_FIXED_POINT16:
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT16:
      return 2;
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT32:
      return 4;
    case platforms::darwinn::DataType_BFLOAT:
      return 2;
    case platforms::darwinn::DataType_HALF:
      return 2;
    case platforms::darwinn::DataType_SINGLE:
      return 4;
    default:
      printf("Invalid DataType passed to TensorDataTypeSize().");
      return 0;
  }
}
}  // namespace

namespace coralmicro {

EdgeTpuExecutable::EdgeTpuExecutable(const platforms::darwinn::Executable* exe)
    : executable_(exe) {
  if (executable_->output_layers()) {
    for (const auto* output_layer : *(executable_->output_layers())) {
      output_layers_[output_layer->name()->c_str()] =
          new OutputLayer(output_layer);
    }
  }
}

EdgeTpuExecutable::~EdgeTpuExecutable() {
  for (auto entry : output_layers_) {
    delete entry.second;
  }
}

#define RETURN_IF_ERROR(expr) \
  do {                        \
    bool ret = expr;          \
    if (!ret) {               \
      return kTfLiteError;    \
    }                         \
  } while (0);

TfLiteStatus EdgeTpuExecutable::Invoke(const TpuDriver& tpu_driver,
                                       TfLiteContext* context,
                                       TfLiteNode* node) {
  const TfLiteEvalTensor* input_tensor =
      tflite::micro::GetEvalInput(context, node, 0);
  const int input_size = tflite::micro::GetTensorShape(input_tensor).FlatSize();
  if (!input_tensor) {
    return kTfLiteError;
  }

  const platforms::darwinn::DmaDescriptorHint* dma_hint;
  const char* name;
  uint8_t* output;
  int32_t ins_idx;
  const flatbuffers::Vector<uint8_t>* bitstream;

  for (const auto* hint : *(executable_->dma_hints()->hints())) {
    switch (hint->any_hint_type()) {
      case platforms::darwinn::AnyHint_DmaDescriptorHint:
        dma_hint = hint->any_hint_as_DmaDescriptorHint();
        switch (dma_hint->meta()->desc()) {
          case platforms::darwinn::Description_BASE_ADDRESS_PARAMETER:
            RETURN_IF_ERROR(tpu_driver.SendParameters(
                executable_->parameters()->data() + dma_hint->offset_in_bytes(),
                dma_hint->size_in_bytes()));
            break;
          case platforms::darwinn::Description_BASE_ADDRESS_INPUT_ACTIVATION:
            name = dma_hint->meta()->name()->c_str();
            if (executable_->input_layers()) {
              for (const auto* input_layer : *(executable_->input_layers())) {
                if (!strcmp(input_layer->name()->c_str(), name)) {
                  if (OutputLayer::SignedDataType(input_layer->data_type())) {
                    OutputLayer::TransformSignedDataType(
                        input_tensor->data.uint8, input_size,
                        TensorDataTypeSize(input_layer->data_type()),
                        input_layer->x_dim(), input_layer->y_dim(),
                        input_layer->z_dim());
                  }
                }
              }
            }
            RETURN_IF_ERROR(tpu_driver.SendInputs(
                input_tensor->data.uint8 + dma_hint->offset_in_bytes(),
                dma_hint->size_in_bytes()));
            break;
          case platforms::darwinn::Description_BASE_ADDRESS_OUTPUT_ACTIVATION:
            name = dma_hint->meta()->name()->c_str();
            if (output_layers_.find(name) == output_layers_.end()) {
              printf("Executable does not have output layer %s\r\n", name);
              break;
            }
            output = output_layers_.at(name)->output_buffer();
            RETURN_IF_ERROR(
                tpu_driver.GetOutputs(output, dma_hint->size_in_bytes()));
            break;
          default:
            break;
        }
        break;
      case platforms::darwinn::AnyHint_InstructionHint:
        ins_idx =
            hint->any_hint_as_InstructionHint()->instruction_chunk_index();
        bitstream =
            executable_->instruction_bitstreams()->Get(ins_idx)->bitstream();
        RETURN_IF_ERROR(
            tpu_driver.SendInstructions(bitstream->data(), bitstream->size()));
        break;
      default:
        break;
    }
  }

  tpu_driver.ReadEvent();

  if (!output_layers_.empty()) {
    for (int i = 0; i < node->outputs->size; ++i) {
      const TfLiteEvalTensor* output_tensor =
          tflite::micro::GetEvalOutput(context, node, i);
      const int output_size =
          tflite::micro::GetTensorShape(output_tensor).FlatSize();
      if (!output_tensor) {
        return kTfLiteError;
      }
      name = executable_->output_layers()->Get(i)->name()->c_str();
      if (output_layers_.find(name) == output_layers_.end()) {
        printf("Executable does not have buffer for %s\r\n", name);
        return kTfLiteError;
      }
      OutputLayer* output_layer = output_layers_[name];

      output_layer->Relayout(output_tensor->data.uint8);
      output_layer->TransformSignedDataType(output_tensor->data.uint8,
                                            output_size);
    }
  }

  return kTfLiteOk;
}

int OutputLayer::DataTypeSize() const {
  return TensorDataTypeSize(output_layer_->data_type());
}

bool OutputLayer::SignedDataType() const {
  return SignedDataType(output_layer_->data_type());
}

bool OutputLayer::SignedDataType(platforms::darwinn::DataType type) {
  switch (type) {
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT8:
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT16:
      return true;

    case platforms::darwinn::DataType_FIXED_POINT8:
    case platforms::darwinn::DataType_FIXED_POINT16:
      // TODO(b/136014872): DataType_SIGNED_FIXED_POINT32 (previously
      // DataType_FIXED_POINT32) is a signed number, see b/135944737.
      // However, the function returns false, which looks like a bug.
      // Please confirm it.
    case platforms::darwinn::DataType_SIGNED_FIXED_POINT32:
    case platforms::darwinn::DataType_BFLOAT:
    case platforms::darwinn::DataType_HALF:
    case platforms::darwinn::DataType_SINGLE:
      return false;
  }

  return false;
}

void OutputLayer::Relayout(uint8_t* dest) const {
  uint8_t* src = output_buffer_.get();
  const auto data_type_size = DataTypeSize();
  const int z_bytes = z_dim() * data_type_size;

  if (y_dim() == 1 && x_dim() == 1) {
    // One dimensional output (only z-dimension).
    if (src != dest) {
      const int padded_size_bytes = PaddedSizeBytes();
      const int actual_size_bytes = ActualSizeBytes();
      const int executions = execution_count_per_inference();
      if (executions == 1 || padded_size_bytes == actual_size_bytes) {
        memcpy(dest, src, z_bytes * executions);
      } else {
        // Remove padding values at the end of each execution.
        const int padded_size_per_execution =
            (padded_size_bytes - actual_size_bytes) / executions;
        for (int i = 0; i < executions; ++i) {
          memcpy(dest, src, z_bytes);
          dest += z_bytes;
          src += z_bytes + padded_size_per_execution;
        }
      }
    }
  } else {
    int z_bytes_padded;
    if (x_dim() > 1) {
      // If x-dim is > 1, padded-z-size can be deduced by looking at
      // difference between offset of element y=0,x=0,z=0 and y=0,x=1,z=0.
      z_bytes_padded = GetBufferIndex(0, 1, 0) - GetBufferIndex(0, 0, 0);
    } else {
      // Otherwise when x-dim is 1 (y-dim must be > 1 in that case),
      // padded-z-size can be deduced by looking at difference between
      // offset of element y=0,x=0,z=0 and y=1,x=0,z=0.
      z_bytes_padded = GetBufferIndex(1, 0, 0) - GetBufferIndex(0, 0, 0);
    }
    z_bytes_padded *= data_type_size;

    const auto* layout = output_layer_->any_layer_as_OutputLayer()->layout();
    int last_x = 0;
    size_t active_tile_x_count = 0;

    int last_x_tile = layout->x_coordinate_to_linear_tile_id_map()->Get(0);

    for (int x = 1; x < x_dim(); ++x) {
      int cur_x_tile = layout->x_coordinate_to_linear_tile_id_map()->Get(x);
      if (cur_x_tile != last_x_tile) {
        active_tile_x_sizes_[active_tile_x_count] = x - last_x;
        active_tile_x_count++;
        last_x_tile = cur_x_tile;
        last_x = x;
      }
    }
    active_tile_x_sizes_[active_tile_x_count] = x_dim() - last_x;
    active_tile_x_count++;

    // When the num_z_bytes parameter is a compile-time constant, the
    // conditions in the innermost loop will be replaced with a single
    // optimized path, specialized for that value. Specialization is
    // provided for num_z_bytes value of 1 and 3. We can also make this a
    // helper function and still realize the benefits provided we have a
    // guaranteed way of ensuring this function would be inlined so that the
    // compiler optimizations based on compile-time-constants can kick in.
#define RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(num_z_bytes, num_z_bytes_padded) \
  do {                                                                        \
    for (int y = 0; y < y_dim(); ++y) {                                       \
      const auto y_buffer_index = GetYBufferIndex(y);                         \
      int tile_starting_x = 0;                                                \
      for (size_t x_tile = 0; x_tile < active_tile_x_count; ++x_tile) {       \
        const unsigned char* source =                                         \
            src + GetBufferIndex(y_buffer_index, tile_starting_x, 0) *        \
                      data_type_size;                                         \
        const int tile_x_size = active_tile_x_sizes_[x_tile];                 \
        for (int local_offset_x = 0; local_offset_x < tile_x_size;            \
             ++local_offset_x) {                                              \
          if ((num_z_bytes) == 1) {                                           \
            *dest = *source;                                                  \
          } else if ((num_z_bytes) == 3) {                                    \
            *(dest + 0) = *(source + 0);                                      \
            *(dest + 1) = *(source + 1);                                      \
            *(dest + 2) = *(source + 2);                                      \
          } else {                                                            \
            memcpy(dest, source, (num_z_bytes));                              \
          }                                                                   \
          dest += (num_z_bytes);                                              \
          source += (num_z_bytes_padded);                                     \
        }                                                                     \
        tile_starting_x += tile_x_size;                                       \
      }                                                                       \
    }                                                                         \
  } while (0)

    if (z_bytes == 1) {
      // Specialization for z_bytes = 1 (grayscale image).
      RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(1, 4);
    } else if (z_bytes == 3) {
      // Specialization for z_bytes = 3 (RGB image).
      RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(3, 4);
    } else {
      // Default.
      RELAYOUT_WITH_Z_BYTES_SPECIALIZATION(z_bytes, z_bytes_padded);
    }

#undef RELAYOUT_WITH_Z_BYTES_SPECIALIZATION
  }
}

void OutputLayer::TransformSignedDataType(uint8_t* buffer, int buffer_size,
                                          int data_type_size, int x_dim,
                                          int y_dim, int z_dim) {
  int buffer_index = 0;

  for (int y = 0; y < y_dim; ++y) {
    for (int x = 0; x < x_dim; ++x) {
      for (int z = 0; z < z_dim; ++z) {
        // XORing with 128 on the last byte of each entry will flip the
        // MSB of each entry. Please note that bytes are stored little
        // endian.
        int msb_index = buffer_index + data_type_size - 1;
        buffer[msb_index] = buffer[msb_index] ^ 128;
        buffer_index += data_type_size;
      }
    }
  }
}

void OutputLayer::TransformSignedDataType(uint8_t* buffer,
                                          int buffer_size) const {
  if (!SignedDataType()) {
    return;
  }

  if (buffer_size < ActualSizeBytes()) {
    printf("Provided buffer size is less than actual size_bytes.");
    return;
  }

  TransformSignedDataType(buffer, buffer_size, DataTypeSize(), x_dim(), y_dim(),
                          z_dim());
}

// Used in GetBufferIndex(int y, int x, int z)
OutputLayer::YBufferIndex OutputLayer::GetYBufferIndex(int y) const {
  const auto& layout = output_layer_->any_layer_as_OutputLayer()->layout();
  YBufferIndex output;
  output.y_linearized_tile_id =
      layout->y_coordinate_to_linear_tile_id_map()->Get(y);
  output.local_y_coordinate = layout->y_coordinate_to_local_y_offset()->Get(y);
  return output;
}

int OutputLayer::GetBufferIndex(int y, int x, int z) const {
  return GetBufferIndex(GetYBufferIndex(y), x, z);
}

int OutputLayer::GetBufferIndex(const YBufferIndex& y_buffer_index, int x,
                                int z) const {
  const auto& layout = output_layer_->any_layer_as_OutputLayer()->layout();
  const int linear_tile_id =
      y_buffer_index.y_linearized_tile_id +
      layout->x_coordinate_to_linear_tile_id_map()->Get(x);
  const int global_tile_byte_offset =
      layout->linearized_tile_byte_offset()->Get(linear_tile_id);

  const int local_x_byte_offset =
      layout->x_coordinate_to_local_byte_offset()->Get(x);
  const int local_y_byte_offset =
      y_buffer_index.local_y_coordinate *
      layout->x_coordinate_to_local_y_row_size()->Get(x);

  return global_tile_byte_offset + local_y_byte_offset + local_x_byte_offset +
         z;
}

}  // namespace coralmicro
