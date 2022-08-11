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

#include "libs/base/timer.h"

#include <climits>
#include <ctime>
#include <sys/time.h>

#include "libs/base/check.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_hp.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_lp.h"

namespace coralmicro {
namespace {
// Number of times the microseconds counter has rolled over.
uint64_t g_micros_rollover = 0;
}  // namespace

void TimerInit() {
  gpt_config_t gpt_config;
  GPT_GetDefaultConfig(&gpt_config);
  gpt_config.clockSource = kGPT_ClockSource_Periph;

  auto root_freq = CLOCK_GetRootClockFreq(kCLOCK_Root_Gpt1);

  GPT_Init(GPT1, &gpt_config);
  GPT_EnableInterrupts(GPT1, kGPT_RollOverFlagInterruptEnable);
  GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
  // Set the divider to get us close to 1MHz.
  GPT_SetClockDivider(GPT1, root_freq / 1000000);
  EnableIRQ(GPT1_IRQn);
  GPT_StartTimer(GPT1);

  snvs_hp_rtc_config_t hp_rtc_config;
  snvs_lp_srtc_config_t lp_rtc_config;
  SNVS_HP_RTC_GetDefaultConfig(&hp_rtc_config);
  SNVS_LP_SRTC_GetDefaultConfig(&lp_rtc_config);
  SNVS_HP_RTC_Init(SNVS, &hp_rtc_config);
  SNVS_LP_SRTC_Init(SNVS, &lp_rtc_config);
}

uint64_t TimerMicros() {
  return g_micros_rollover * (uint64_t{1} << 32) +
         static_cast<uint64_t>(GPT_GetCurrentTimerCount(GPT1));
}

void TimerGetRtcTime(struct tm* time) {
  snvs_hp_rtc_datetime_t hp_date;
  SNVS_HP_RTC_GetDatetime(SNVS, &hp_date);
  time->tm_year = hp_date.year - 1900;
  time->tm_mon = hp_date.month - 1;
  time->tm_mday = hp_date.day;
  time->tm_hour = hp_date.hour;
  time->tm_min = hp_date.minute;
  time->tm_sec = hp_date.second;
}

void TimerSetRtcTime(uint32_t sec) {
  SNVS_HP_RTC_StopTimer(SNVS);
  SNVS_LP_SRTC_StopTimer(SNVS);

  snvs_lp_srtc_datetime_t lp_date;
  struct tm now;
  time_t time = static_cast<time_t>(sec);
  CHECK(gmtime_r(&time, &now));
  lp_date.year = now.tm_year + 1900;
  lp_date.month = now.tm_mon + 1;
  lp_date.day = now.tm_mday;
  lp_date.hour = now.tm_hour;
  lp_date.minute = now.tm_min;
  lp_date.second = now.tm_sec;

  SNVS_LP_SRTC_SetDatetime(SNVS, &lp_date);
  SNVS_LP_SRTC_StartTimer(SNVS);
  SNVS_HP_RTC_TimeSynchronize(SNVS);
  SNVS_HP_RTC_StartTimer(SNVS);
}

}  // namespace coralmicro

extern "C" void GPT1_IRQHandler() {
  if (GPT_GetStatusFlags(GPT1, kGPT_RollOverFlag)) {
    GPT_ClearStatusFlags(GPT1, kGPT_RollOverFlag);
    ++coralmicro::g_micros_rollover;
  }
}

extern "C" uint32_t vPortGetRunTimeCounterValue() {
  return static_cast<uint32_t>(coralmicro::TimerMicros());
}

extern "C" int _gettimeofday (struct timeval *__p, void *__tz) {
  struct tm now;
  coralmicro::TimerGetRtcTime(&now);
  __p->tv_sec = mktime(&now);
  return 0;
}