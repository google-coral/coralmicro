/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* freertos_tasks_c_additions.h Rev. 1.3 */
#ifndef FREERTOS_TASKS_C_ADDITIONS_H
#define FREERTOS_TASKS_C_ADDITIONS_H

#include <stdint.h>

#if (configUSE_TRACE_FACILITY == 0)
#error "configUSE_TRACE_FACILITY must be enabled"
#endif

#define FREERTOS_DEBUG_CONFIG_MAJOR_VERSION 1
#define FREERTOS_DEBUG_CONFIG_MINOR_VERSION 3

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t FreeRTOSDebugConfig[];

char *const portArch_Name __attribute__((section(".rodata.debug"))) = "Cortex-M7";
const uint8_t FreeRTOSDebugConfig[] __attribute__((section(".rodata.debug"))) =
{
    FREERTOS_DEBUG_CONFIG_MAJOR_VERSION,
    FREERTOS_DEBUG_CONFIG_MINOR_VERSION,
    tskKERNEL_VERSION_MAJOR,
    tskKERNEL_VERSION_MINOR,
    tskKERNEL_VERSION_BUILD,
    configFRTOS_MEMORY_SCHEME,
    offsetof(struct tskTaskControlBlock, pxTopOfStack),
    offsetof(struct tskTaskControlBlock, xStateListItem),
    offsetof(struct tskTaskControlBlock, xEventListItem),
    offsetof(struct tskTaskControlBlock, pxStack),
    offsetof(struct tskTaskControlBlock, pcTaskName),
    offsetof(struct tskTaskControlBlock, uxTCBNumber),
    offsetof(struct tskTaskControlBlock, uxTaskNumber),
    configMAX_TASK_NAME_LEN,
    configMAX_PRIORITIES,
    configENABLE_MPU,
    configENABLE_FPU,
    configENABLE_TRUSTZONE,
	configRUN_FREERTOS_SECURE_ONLY,
	0, 			// 32-bit align
	0, 0, 0, 0	// padding
};

#ifdef __cplusplus
}
#endif

#endif // FREERTOS_TASKS_C_ADDITIONS_H
