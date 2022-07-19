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

#include "libs/base/http_server_handlers.h"

#include <algorithm>
#include <cstring>
#include <numeric>
#include <string>
#include <vector>

#include "libs/base/strings.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coralmicro {
namespace {
void GenerateStatsHtml(std::vector<uint8_t>* html) {
  std::vector<TaskStatus_t> infos(uxTaskGetNumberOfTasks());
  uint32_t total_runtime;
  auto n = uxTaskGetSystemState(infos.data(), infos.size(), &total_runtime);
  std::vector<int> indices(infos.size());
  std::iota(std::begin(indices), std::end(indices), 0);
  std::sort(std::begin(indices), std::end(indices), [&infos](int i, int j) {
    return infos[i].ulRunTimeCounter > infos[j].ulRunTimeCounter;
  });

  StrAppend(html, "<!DOCTYPE html>\r\n");
  StrAppend(html, "<html lang=\"en\">\r\n");
  StrAppend(html, "<head>\r\n");
  StrAppend(html, "<title>Run-time statistics</title>\r\n");
  StrAppend(html, "  <style>\r\n");
  StrAppend(html, "    th,td {padding: 2px;}\r\n");
  StrAppend(html, "  </style>\r\n"), StrAppend(html, "</head>\r\n");
  StrAppend(html, "<body>\r\n");
  StrAppend(html, "  <table>\r\n");
  StrAppend(html,
            "    <tr><th>Task</th><th>Abs Time</th><th>% Time</th></tr>\r\n");
  for (auto i = 0u; i < n; ++i) {
    const auto& info = infos[indices[i]];
    auto name = info.pcTaskName;
    auto runtime = info.ulRunTimeCounter;
    float percent = static_cast<float>(runtime) / (total_runtime / 100.0);
    coralmicro::StrAppend(
        html, "    <tr><td>%s</td><td>%lu</td><td>%.1f%%</td></tr>\r\n", name,
        runtime, percent);
  }
  StrAppend(html, "  </table>\r\n");
  StrAppend(html, "</body>\r\n");
}
}  // namespace

HttpServer::Content FileSystemUriHandler::operator()(const char* uri) {
  const auto prefix_len = std::strlen(prefix);
  if (std::strncmp(uri, prefix, prefix_len) == 0)
    return std::string{uri + prefix_len - 1};
  return {};
}

HttpServer::Content TaskStatsUriHandler::operator()(const char* uri) {
  if (std::strcmp(uri, name) == 0) {
    std::vector<uint8_t> html;
    html.reserve(2048);
    GenerateStatsHtml(&html);
    return html;
  }
  return {};
}

}  // namespace coralmicro
