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

#ifndef LIBS_TPU_EDGETPU_MANAGER_H_
#define LIBS_TPU_EDGETPU_MANAGER_H_

#include <cstdlib>
#include <map>
#include <memory>
#include <optional>

#include "libs/tpu/edgetpu_driver.h"
#include "libs/tpu/edgetpu_executable.h"
#include "libs/tpu/executable_generated.h"
#include "libs/tpu/usb_host_edgetpu.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/tflite-micro/tensorflow/lite/c/common.h"

namespace coralmicro {

// This class is a representation of the Edge TPU device, so there is one
// shared `EdgeTpuContext` used by all model interpreters.
//
// Instances of this should be allocated with `EdgeTpuManager::OpenDevice()`.
//
// The `EdgeTpuContext` can be shared among multiple software components, and
// the life of this object is directly tied to the Edge TPU power, so the
// Edge TPU powers down after the last `EdgeTpuContext` reference leaves scope.
//
// The lifetime of the `EdgeTpuContext` must be longer than all associated
// `tflite::MicroInterpreter` instances.
class EdgeTpuContext {
 public:
  // @cond Do not generate docs
  // Use EdgeTpuManager::OpenDevice() instead.
  EdgeTpuContext();
  ~EdgeTpuContext();
  // @endcond
};

// @cond Do not generate docs
class EdgeTpuPackage {
 public:
  EdgeTpuPackage(const platforms::darwinn::Executable* inference_exe,
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
  // @cond Do not generate docs
  EdgeTpuManager();
  EdgeTpuManager(const EdgeTpuManager&) = delete;
  EdgeTpuManager& operator=(const EdgeTpuManager&) = delete;
  // @endcond

  // Gets a pointer to the `EdgeTpuManager` singleton object.
  static EdgeTpuManager* GetSingleton() {
    static EdgeTpuManager manager;
    return &manager;
  }

  // @cond Do not generate docs
  EdgeTpuPackage* RegisterPackage(const char* package_content, size_t length);
  TfLiteStatus Invoke(EdgeTpuPackage* package, TfLiteContext* context,
                      TfLiteNode* node);
  // @endcond

  // Gets the default Edge TPU device (and starts it if necessary).
  //
  // The Edge TPU device (represented by `EdgeTpuContext`) can be shared among
  // multiple software components, and the `EdgeTpuManager` is a singleton
  // object, so you should always call this function like this:
  //
  // ```
  // auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
  // ```
  //
  // @param mode The `PerformanceMode` to use for the Edge TPU. Options are:
  // `kMax` (500Mhz), `kHigh` (250Mhz), `kMedium` (125Mhz), or `kLow` (63Mhz).
  // If omitted, the default is `kHigh`.
  // **Caution**: If you set the performance mode to `kMax`, it can increase
  // the Edge TPU inferencing speed, but it can also make the Edge TPU
  // module hotter, which might cause burns if touched.
  //
  // @return A shared pointer to Edge TPU device. The shared_ptr can point to
  // nullptr in case of error.
  std::shared_ptr<EdgeTpuContext> OpenDevice(
      PerformanceMode mode = PerformanceMode::kHigh);

  // @cond Do not generate docs
  void NotifyError();
  void NotifyConnected(usb_host_edgetpu_instance_t* usb_instance);
  // @endcond

  // Gets the current Edge TPU junction temperature.
  // @returns The temperature in Celcius, or `std::nullopt` if
  // `EdgeTpuContext` is empty.
  std::optional<float> GetTemperature();

 private:
  TpuDriver tpu_driver_;
  std::map<uintptr_t, EdgeTpuPackage*> packages_;
  std::array<EdgeTpuPackage*, 2> cached_packages_;
  uint64_t current_parameter_caching_token_ = 0;
  usb_host_edgetpu_instance_t* usb_instance_ = nullptr;
  std::weak_ptr<EdgeTpuContext> context_;
  SemaphoreHandle_t mutex_;
  bool usb_error_{false};
};

}  // namespace coralmicro

#endif  // LIBS_TPU_EDGETPU_MANAGER_H_
