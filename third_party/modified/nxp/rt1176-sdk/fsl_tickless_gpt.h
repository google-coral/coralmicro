/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FSL_TICKLESS_GPT_H
#define FSL_TICKLESS_GPT_H

#include "fsl_clock.h"

// Target 2kHz for our sleep timer.
// This is comfortably higher than our tick rate, but
// still pretty slow - so hopefully strikes a good balance
// between accurate timekeeping and allowed duration of sleep.
#define configGPT_CLOCK_HZ (2000)

#if (__CORTEX_M == 7)
#define SLEEP_GPT GPT2
#define SLEEP_GPT_IRQn GPT2_IRQn
#define SLEEP_GPT_IRQHandler GPT2_IRQHandler
#else
#define SLEEP_GPT GPT3
#define SLEEP_GPT_IRQn GPT3_IRQn
#define SLEEP_GPT_IRQHandler GPT3_IRQHandler
#endif

/* The GPT is a 32-bit counter. */
#define portMAX_32_BIT_NUMBER (0xffffffffUL)

#endif /* FSL_TICKLESS_GPT_H */
