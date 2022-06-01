#ifndef LIBS_TPU_EDGETPU_MANAGER_H_
#define LIBS_TPU_EDGETPU_MANAGER_H_

#include <cstdlib>
#include <map>
#include <memory>

#include "libs/tpu/edgetpu_executable.h"
#include "libs/tpu/executable_generated.h"
#include "libs/tpu/tpu_driver.h"
#include "libs/usb_host_edgetpu/usb_host_edgetpu.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coral::micro {

// This class is essentially a representation of the Edge TPU, so there is one
// shared `EdgeTpuContext` used by all model interpreters. Instances of this
// class should always be allocated with `EdgeTpuManager::OpenDevice()`.
//
// Unlike the libcoral C++ API, when using this coralmicro C++ API, you do not
// need to pass the `EdgeTpuContext` to the `tflite::MicroInterpreter`, but the
// context must be opened and the custom op must be registered before you create
// an interpreter. (This is different because libcoral is based on TensorFlow
// Lite and coralmicro is based on TensorFlow Lite for Microcontrollers.)
//
// For example:
//
// ```
// auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
// if (!tpu_context) {
//     printf("ERROR: Failed to get EdgeTpu context\r\n");
//     return;
// }
//
// tflite::MicroErrorReporter error_reporter;
// tflite::MicroMutableOpResolver<1> resolver;
// resolver.AddCustom(kCustomOp, RegisterCustomOp());
//
// auto tensor_arena = std::make_unique<TensorArena>();
// tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()),
//                                      resolver, tensor_arena->data,
//                                      kTensorArenaSize, &error_reporter);
// ```
//
// To see the rest of this code example, see
// `examples/classify_image/classify_image.cc`.
//
// The lifetime of the Edge TPU context must be longer than all associated
// `tflite::MicroInterpreter` instances.
//
// The life of this object is directly tied to the Edge TPU power. So when this
// object is destroyed, the Edge TPU powers down.
class EdgeTpuContext {
   public:
    EdgeTpuContext();
    ~EdgeTpuContext();
};

// @cond Internal only, do not generate docs
class EdgeTpuPackage {
   public:
    EdgeTpuPackage(
        const platforms::darwinn::Executable* inference_exe,
        const platforms::darwinn::Executable* parameter_caching_exe) {
        inference_ = std::make_unique<EdgeTpuExecutable>(inference_exe);
        if (parameter_caching_exe) {
            parameter_caching_ =
                std::make_unique<EdgeTpuExecutable>(parameter_caching_exe);
        }
    }
    EdgeTpuExecutable* parameter_caching_exe() {
        return parameter_caching_.get();
    }
    EdgeTpuExecutable* inference_exe() { return inference_.get(); }

   private:
    std::unique_ptr<EdgeTpuExecutable> inference_;
    std::unique_ptr<EdgeTpuExecutable> parameter_caching_;
};
// @endcond

// Singleton Edge TPU manager for allocating new instances of `EdgeTpuContext`.
class EdgeTpuManager {
   public:
    EdgeTpuManager();
    EdgeTpuManager(const EdgeTpuManager&) = delete;
    EdgeTpuManager& operator=(const EdgeTpuManager&) = delete;

    // Gets a pointer to the EdgeTpuManager singleton object.
    static EdgeTpuManager* GetSingleton() {
        static EdgeTpuManager manager;
        return &manager;
    }

    // @cond Internal only, do not generate docs
    EdgeTpuPackage* RegisterPackage(const char* package_content, size_t length);
    TfLiteStatus Invoke(EdgeTpuPackage* package, TfLiteContext* context,
                        TfLiteNode* node);
    // @endcond

    // Opens the default Edge TPU device.
    //
    // The Edge TPU device (represented as `EdgeTpuContext`) can be shared among
    // multiple software components. The device is closed after the last
    // reference leaves scope.
    //
    // @param mode The `PerformanceMode` to use for the Edge TPU. Options are:
    // `kMax` (500Mhz), `kHigh` (250Mhz), `kMedium` (125Mhz), or `kLow` (63Mhz).
    // If omitted, the default is `kHigh`.
    //
    // @return A shared pointer to Edge TPU device. The shared_ptr can point to
    // nullptr in case of error.
    std::shared_ptr<EdgeTpuContext> OpenDevice(
        PerformanceMode mode = PerformanceMode::kHigh);

    // @cond Internal only, do not generate docs
    void NotifyConnected(usb_host_edgetpu_instance_t* usb_instance);
    // @endcond

    // Gets the current Edge TPU junction temperature.
    // @returns The temperature in Celcius, or -276.88 if the `EdgeTpuContext`
    //   is empty.
    float GetTemperature();

   private:
    TpuDriver tpu_driver_;
    std::map<uintptr_t, EdgeTpuPackage*> packages_;
    std::array<EdgeTpuPackage*, 2> cached_packages_;
    uint64_t current_parameter_caching_token_ = 0;
    usb_host_edgetpu_instance_t* usb_instance_ = nullptr;
    std::weak_ptr<EdgeTpuContext> context_;
    SemaphoreHandle_t mutex_;
};

}  // namespace coral::micro

#endif  // LIBS_TPU_EDGETPU_MANAGER_H_
