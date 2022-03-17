#ifndef _LIBS_TPU_EDGETPU_EXECUTABLE_H_
#define _LIBS_TPU_EDGETPU_EXECUTABLE_H_

#include "libs/tpu/executable_generated.h"
#include "libs/tpu/tpu_driver.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

#include <cstdlib>
#include <cstring>
#include <map>

namespace valiant {

class OutputLayer {
  public:
    explicit OutputLayer(const platforms::darwinn::Layer* layer) :
        output_layer_(layer),
        output_buffer_(std::make_unique<uint8_t[]>(layer->size_bytes())),
        active_tile_x_sizes_(std::make_unique<int[]>(x_dim())) {}
    OutputLayer(const OutputLayer&) = delete;
    OutputLayer& operator=(const OutputLayer&) = delete;
    uint8_t *output_buffer() { return output_buffer_.get(); }

    void Relayout(uint8_t *dest) const;
    void TransformSignedDataType(uint8_t *buffer, int buffer_size) const;
  private:
    struct YBufferIndex {
        // Holds the linearized tile ID for a given y value.
        int y_linearized_tile_id;
        // Holds local offset within a data chunk returned by a given tile.
        int local_y_coordinate;
    };
    YBufferIndex GetYBufferIndex(int y) const;
    int GetBufferIndex(int y, int x, int z) const;
    int GetBufferIndex(const YBufferIndex& y_buffer_index, int x, int z) const;

    int execution_count_per_inference() const {
        return output_layer_->execution_count_per_inference();
    }
    int ActualSizeBytes() const {
        const int num_elements = x_dim() * y_dim() * z_dim();
        return num_elements * DataTypeSize() * execution_count_per_inference();
    }

    int PaddedSizeBytes() const {
        return output_layer_->size_bytes() * execution_count_per_inference();
    }

    int DataTypeSize() const;
    bool SignedDataType() const;
    int x_dim() const { return output_layer_->x_dim(); }
    int y_dim() const { return output_layer_->y_dim(); }
    int z_dim() const { return output_layer_->z_dim(); }

    const platforms::darwinn::Layer *output_layer_;
    std::unique_ptr<uint8_t[]> output_buffer_;
    std::unique_ptr<int[]> active_tile_x_sizes_;
};

class EdgeTpuExecutable {
  public:
    explicit EdgeTpuExecutable(const platforms::darwinn::Executable *exe);
    EdgeTpuExecutable(const EdgeTpuExecutable&) = delete;
    EdgeTpuExecutable& operator=(const EdgeTpuExecutable&) = delete;

    TfLiteStatus Invoke(const TpuDriver& tpu_driver, TfLiteContext* context, TfLiteNode *node);

    uint64_t ParameterCachingToken() {
      return executable_->parameter_caching_token();
    }
  private:
    const platforms::darwinn::Executable* executable_;

    struct Less {
      bool operator()(const char* a, const char* b) const {
        return std::strcmp(a, b) < 0;
      }
    };
    std::map<const char*, OutputLayer*, Less> output_layers_;
};

}  // namespace valiant

#endif  // _LIBS_TPU_EDGETPU_EXECUTABLE_H_
