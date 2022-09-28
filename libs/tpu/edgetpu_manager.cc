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

#include "libs/tpu/edgetpu_manager.h"

#include <cstdio>

#include "libs/base/check.h"
#include "libs/base/mutex.h"
#include "libs/tpu/edgetpu_task.h"
#include "third_party/flatbuffers/include/flatbuffers/flatbuffers.h"
#include "third_party/flatbuffers/include/flatbuffers/flexbuffers.h"
#include "third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction.h"

namespace coralmicro {
namespace {
constexpr char kKeyVersion[] = "1";
constexpr char kKeyChipName[] = "2";
constexpr char kKeyParamCache_DEPRECATED[] = "3";
constexpr char kKeyExecutable[] = "4";
}  // namespace

EdgeTpuContext::EdgeTpuContext() {
  EdgeTpuTask::GetSingleton()->SetPower(true);
}

EdgeTpuContext::~EdgeTpuContext() {
  EdgeTpuTask::GetSingleton()->SetPower(false);
  // Small delay ensuring usb instance is released.
  vTaskDelay(pdMS_TO_TICKS(30));
}

EdgeTpuManager::EdgeTpuManager() : mutex_(xSemaphoreCreateMutex()) {
  CHECK(mutex_);
}

void EdgeTpuManager::NotifyConnected(
    usb_host_edgetpu_instance_t* usb_instance) {
  usb_instance_ = usb_instance;

  // The EdgeTPU has left the USB bus -- clean up state.
  if (!usb_instance_) {
    current_parameter_caching_token_ = 0;
  }
}

void EdgeTpuManager::NotifyError() { usb_error_ = true; }

std::shared_ptr<EdgeTpuContext> EdgeTpuManager::OpenDevice(
    PerformanceMode mode) {
  MutexLock lock(mutex_);

  auto context = context_.lock();
  if (context) return context;

  context = std::make_shared<EdgeTpuContext>();

  while (!usb_instance_) {
    if (usb_error_) {
      printf("%s: Error encountered while bringing up the tpu\r\n", __func__);
      usb_error_ = false;  // Reset error.
      return nullptr;
    }
    taskYIELD();
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Got tpu usb instance, init the tpu driver.
  if (!tpu_driver_.Initialize(usb_instance_, mode)) {
    return nullptr;
  }

  context_ = context;
  return context;
}

EdgeTpuPackage* EdgeTpuManager::RegisterPackage(const char* package_content,
                                                size_t length) {
  MutexLock lock(mutex_);
  auto package_ptr = (uintptr_t)package_content;

  if (packages_.find(package_ptr) != packages_.end()) {
    return packages_[package_ptr];
  }

  auto flexbuffer_map =
      flexbuffers::GetRoot((const uint8_t*)package_ptr, length).AsMap();
  auto package_binary = flexbuffer_map[kKeyExecutable].AsString();
  flatbuffers::Verifier package_verifier((const uint8_t*)package_binary.c_str(),
                                         package_binary.length());
  if (!package_verifier.VerifyBuffer<platforms::darwinn::Package>()) {
    printf("Package verification failed.\r\n");
    return nullptr;
  }

  auto* package =
      flatbuffers::GetRoot<platforms::darwinn::Package>(package_binary.c_str());
  if (flatbuffers::VectorLength(package->serialized_multi_executable()) == 0) {
    printf("No executables to register.\r\n");
    return nullptr;
  }

  auto* multi_executable =
      flatbuffers::GetRoot<platforms::darwinn::MultiExecutable>(
          package->serialized_multi_executable()->data());
  flatbuffers::Verifier multi_executable_verifier(
      package->serialized_multi_executable()->data(),
      flatbuffers::VectorLength(package->serialized_multi_executable()));
  if (!multi_executable_verifier
           .VerifyBuffer<platforms::darwinn::MultiExecutable>()) {
    printf("MultiExecutable verification failed.\r\n");
    return nullptr;
  }

  const platforms::darwinn::Executable* inference_exe = nullptr;
  const platforms::darwinn::Executable* parameter_caching_exe = nullptr;

  for (const auto* executable_serialized :
       *(multi_executable->serialized_executables())) {
    flatbuffers::Verifier verifier(
        (const uint8_t*)executable_serialized->c_str(),
        executable_serialized->size());
    if (!verifier.VerifyBuffer<platforms::darwinn::Executable>()) {
      printf("Executable verification failed.\r\n");
      return nullptr;
    }

    const auto* executable =
        flatbuffers::GetRoot<platforms::darwinn::Executable>(
            (const uint8_t*)executable_serialized->c_str());
    if (executable->type() ==
            platforms::darwinn::ExecutableType_EXECUTION_ONLY ||
        executable->type() == platforms::darwinn::ExecutableType_STAND_ALONE) {
      inference_exe = executable;
    } else if (executable->type() ==
               platforms::darwinn::ExecutableType_PARAMETER_CACHING) {
      parameter_caching_exe = executable;
    }
  }

  if (inference_exe == nullptr) {
    printf("Package does not have inference executable.\r\n");
    return nullptr;
  }

  auto* edgetpu_package =
      new EdgeTpuPackage(inference_exe, parameter_caching_exe);
  packages_[package_ptr] = edgetpu_package;

  return edgetpu_package;
}

TfLiteStatus EdgeTpuManager::Invoke(EdgeTpuPackage* package,
                                    TfLiteContext* context, TfLiteNode* node) {
  MutexLock lock(mutex_);
  if (package->parameter_caching_exe()) {
    auto token = package->parameter_caching_exe()->ParameterCachingToken();
    if (token != current_parameter_caching_token_) {
      cached_packages_.fill(nullptr);
      package->parameter_caching_exe()->Invoke(tpu_driver_, context, node);
      current_parameter_caching_token_ = token;
      cached_packages_[0] = package;
    } else {
      for (auto& cached_package : cached_packages_) {
        if (cached_package == package) {
          break;
        } else if (cached_package == nullptr) {
          package->parameter_caching_exe()->Invoke(tpu_driver_, context, node);
          cached_package = package;
        }
      }
    }
  } else {
    current_parameter_caching_token_ = 0;
  }

  return package->inference_exe()->Invoke(tpu_driver_, context, node);
}

std::optional<float> EdgeTpuManager::GetTemperature() {
  MutexLock lock(mutex_);
  // Only attempt to read the temperature if the device has been opened.
  auto context = context_.lock();
  if (context) return tpu_driver_.GetTemperature();
  return std::nullopt;
}

}  // namespace coralmicro
