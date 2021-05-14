#include "libs/base/tasks.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.h"

#include <cstdio>

#define BUF_LENGTH (0x8000)
uint8_t src_data[BUF_LENGTH] __attribute__((aligned(32))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t dst_data[BUF_LENGTH] __attribute__((aligned(32))) __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds) {
    BaseType_t reschedule = pdFALSE;
    SemaphoreHandle_t sema = reinterpret_cast<SemaphoreHandle_t>(param);
    if (transferDone) {
        xSemaphoreGiveFromISR(sema, &reschedule);
        portYIELD_FROM_ISR(reschedule);
    }
}

extern "C" void app_main(void *param) {
    printf("DMATest %p->%p\r\n", src_data, dst_data);
    for (int i = 0; i < BUF_LENGTH; ++i) {
        src_data[i] = i & 0xFF;
        dst_data[i] = 0xAA;
    }
    DCACHE_InvalidateByRange(reinterpret_cast<uint32_t>(src_data), sizeof(src_data));
    DCACHE_InvalidateByRange(reinterpret_cast<uint32_t>(dst_data), sizeof(dst_data));

    SemaphoreHandle_t edma_semaphore = xSemaphoreCreateBinary();

    NVIC_SetPriority(DMA0_DMA16_IRQn, 6);

    DMAMUX_Init(DMAMUX0);
    DMAMUX_EnableAlwaysOn(DMAMUX0, 0, true);
    DMAMUX_EnableChannel(DMAMUX0, 0);

    edma_handle_t dma_handle;
    edma_config_t dma_config;
    edma_transfer_config_t transfer_config;
    EDMA_GetDefaultConfig(&dma_config);
    EDMA_Init(DMA0, &dma_config);
    EDMA_CreateHandle(&dma_handle, DMA0, 0);
    EDMA_SetCallback(&dma_handle, EDMA_Callback, edma_semaphore);
    EDMA_PrepareTransfer(&transfer_config, src_data, sizeof(uint32_t), dst_data, sizeof(uint32_t),
            sizeof(uint32_t), sizeof(src_data), kEDMA_MemoryToMemory);
    EDMA_SubmitTransfer(&dma_handle, &transfer_config);
    EDMA_StartTransfer(&dma_handle);

    printf("Wait for DMA completion\r\n");
    xSemaphoreTake(edma_semaphore, portMAX_DELAY);
    DCACHE_InvalidateByRange(reinterpret_cast<uint32_t>(dst_data), sizeof(dst_data));

    for (size_t i = 0; i < sizeof(src_data); ++i) {
        if (src_data[i] != dst_data[i]) {
            printf("Mismatch at %d: 0x%x != 0x%x\r\n", i, src_data[i], dst_data[i]);
            break;
        }
    }

    printf("DMATest done\r\n");
    while (true) {
        taskYIELD();
    }
}
