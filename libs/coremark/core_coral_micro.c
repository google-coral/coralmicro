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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "core_portme.h"

static char *printf_buf;
static int current_buffer_length;
static bool print_to_console = true;

int ee_printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  /* Use stdio printf for console output */
  if (print_to_console) {
    vprintf(fmt, args);
    printf("\r");
  }
  /* Append message to buffer */
  current_buffer_length +=
      vsnprintf(printf_buf + current_buffer_length,
                MAX_COREMARK_BUFFER - current_buffer_length, fmt, args);
  current_buffer_length +=
      vsnprintf(printf_buf + current_buffer_length,
                MAX_COREMARK_BUFFER - current_buffer_length, "\r", args);
  va_end(args);
  return 0;
}

void CoreMark_PrintToConsole(bool print) { print_to_console = print; }

void RunCoreMark(char *buffer) {
  printf_buf = buffer;
  current_buffer_length = 0;
  coremark_main();
}
