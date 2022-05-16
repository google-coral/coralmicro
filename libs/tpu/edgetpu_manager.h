#ifndef LIBS_TPU_EDGETPU_MANAGER_H_
#define LIBS_TPU_EDGETPU_MANAGER_H_

#include "libs/tpu/edgetpu_executable.h"
#include "libs/tpu/executable_generated.h"
#include "libs/tpu/tpu_driver.h"
#include "libs/usb_host_edgetpu/usb_host_edgetpu.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"

#include <cstdlib>
#include <map>
#include <memory>

namespace coral::micro {

class EdgeTpuContext {
  public:
    EdgeTpuContext();
    ~EdgeTpuContext();
};

class EdgeTpuPackage {
  public:
    EdgeTpuPackage(const platforms::darwinn::Executable* inference_exe,
                   const platforms::darwinn::Executable* parameter_caching_exe) {
        inference_ = std::make_unique<EdgeTpuExecutable>(inference_exe);
        if (parameter_caching_exe) {
            parameter_caching_ = std::make_unique<EdgeTpuExecutable>(parameter_caching_exe);
        }
    }
    EdgeTpuExecutable* parameter_caching_exe() {
        return parameter_caching_.get();
    }
    EdgeTpuExecutable* inference_exe() {
        return inference_.get();
    }
  private:
    std::unique_ptr<EdgeTpuExecutable> inference_;
    std::unique_ptr<EdgeTpuExecutable> parameter_caching_;
};

class EdgeTpuManager {
  public:
    EdgeTpuManager();
    EdgeTpuManager(const EdgeTpuManager&) = delete;
    EdgeTpuManager& operator=(const EdgeTpuManager&) = delete;

    static EdgeTpuManager* GetSingleton() {
        static EdgeTpuManager manager;
        return &manager;
    }

    EdgeTpuPackage* RegisterPackage(const char* package_content, size_t length);
    TfLiteStatus Invoke(EdgeTpuPackage* package, TfLiteContext *context, TfLiteNode *node);
    std::shared_ptr<EdgeTpuContext> OpenDevice(const PerformanceMode mode);
    std::shared_ptr<EdgeTpuContext> OpenDevice();
    void NotifyConnected(usb_host_edgetpu_instance_t* usb_instance);
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
