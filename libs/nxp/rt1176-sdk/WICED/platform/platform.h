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

#ifndef LIBS_NXP_RT1176_SDK_WICED_PLATFORM_PLATFORM_H_
#define LIBS_NXP_RT1176_SDK_WICED_PLATFORM_PLATFORM_H_

typedef enum {
    WICED_GPIO_MAX,
} wiced_gpio_t;

typedef enum {
    WICED_PWM_MAX,
} wiced_pwm_t;

typedef enum {
    WICED_SPI_MAX,
} wiced_spi_t;

typedef enum {
    WICED_UART_0,
    WICED_UART_MAX,
} wiced_uart_t;
#define STDIO_UART WICED_UART_0

typedef enum {
    WICED_ADC_MAX,
} wiced_adc_t;

typedef enum {
    WICED_I2C_MAX,
} wiced_i2c_t;

#endif  // LIBS_NXP_RT1176_SDK_WICED_PLATFORM_PLATFORM_H_
