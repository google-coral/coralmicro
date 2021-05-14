#include "apps/FlexSPI_NAND/flexspi_nand_lut.h"
#include "libs/base/tasks.h"
#include "libs/nxp/rt1176-sdk/board.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_flexspi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"

#include <cstdio>

void flexspi_init() {
#if defined(__DCACHE_PRESENT)
    bool DCacheEnableFlag = false;
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR)) {
        SCB_DisableDCache();
        DCacheEnableFlag = true;
    }
#endif

    // TODO(atv): Check this divider.
    CLOCK_SetRootClockDiv(kCLOCK_Root_Flexspi1, 2);
    CLOCK_SetRootClockMux(kCLOCK_Root_Flexspi1, 0);

    flexspi_config_t config;
    FLEXSPI_GetDefaultConfig(&config);
    config.ahbConfig.enableAHBPrefetch    = true;
    config.ahbConfig.enableAHBBufferable  = true;
    config.ahbConfig.enableReadAddressOpt = true;
    config.ahbConfig.enableAHBCachable    = true;
    config.rxSampleClock                  = kFLEXSPI_ReadSampleClkLoopbackInternally;
    FLEXSPI_Init(FLEXSPI1, &config);

    flexspi_device_config_t device_config = {
        .flexspiRootClk       = 12000000,
    //    .flashSize            = BOARD_FLASH_SIZE,
        .flashSize = 0x20000,
        .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
        .CSInterval           = 2,
        .CSHoldTime           = 3,
        .CSSetupTime          = 3,
        .dataValidTime        = 0,
        .columnspace          = 16, // TODO(atv): is this 16 or 12?
        .enableWordAddress    = 0,
        .AWRSeqIndex          = 0,
        .AWRSeqNumber         = 0,
        .ARDSeqIndex          = LUT_READ_FROM_CACHE,
        .ARDSeqNumber         = 1,
        .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
        .AHBWriteWaitInterval = 0,
    };
    FLEXSPI_SetFlashConfig(FLEXSPI1, &device_config, kFLEXSPI_PortA1);

    FLEXSPI_UpdateLUT(FLEXSPI1, 0, customLUT, 64);

    FLEXSPI_SoftwareReset(FLEXSPI1);

#if defined(__DCACHE_PRESENT)
    if (DCacheEnableFlag) {
        SCB_EnableDCache();
    }
#endif
}

void flexspi_get_vendor_id(uint16_t *vendor_id) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Read;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_READID;
    xfer.data = (uint32_t*)vendor_id;
    xfer.dataSize = 2;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);

    // FLEXSPI_SoftwareReset(FLEXSPI1);

    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
}

bool flexspi_write_enable() {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Command;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_WRITE_ENABLE;

    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
    return (status == kStatus_Success);
}

void flexspi_read_status(uint8_t *status_out) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Read;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_READ_STATUS;
    xfer.data = (uint32_t*)status_out;
    xfer.dataSize = 1;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);

    printf("%s status: %ld %d 0x%x\r\n", __func__, status, kStatus_Success, *status_out);
}
void flexspi_read_lock_status(uint8_t *status_out) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Read;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_READ_LOCK_STATUS;
    xfer.data = (uint32_t*)status_out;
    xfer.dataSize = 1;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);

    printf("%s status: %ld %d 0x%x\r\n", __func__, status, kStatus_Success, *status_out);
}

void flexspi_erase_block(uint32_t address) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = address;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Command;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_BLOCK_ERASE;

    if (!flexspi_write_enable()) {
        printf("Write enable failed\r\n");
        return;
    }
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
    uint8_t status_reg;
    do {
        flexspi_read_status(&status_reg);
    } while (status_reg & 1);
    printf("erase block finished\r\n");
}

void flexspi_program_load(uint32_t address, uint8_t* data, size_t len) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = address;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Write;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_PROGRAM_LOAD;
    xfer.data = (uint32_t*)data;
    xfer.dataSize = len;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
}

void flexspi_program_execute(uint32_t address) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = address;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Command;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_PROGRAM_EXECUTE;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
}

void flexspi_page_write(uint32_t address, uint8_t* data, size_t len) {
    // write enable
    if (!flexspi_write_enable()) {
        printf("Write enable failed\r\n");
        return;
    }
    // program load
    flexspi_program_load(address, data, len);
    // program execute
    flexspi_program_execute(address);
    // status
    uint8_t status_reg;
    do {
        flexspi_read_status(&status_reg);
    } while (status_reg & 1);
    printf("write page finished\r\n");
}

bool flexspi_page_read_cache(uint32_t address) {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = address;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Command;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_READ_PAGE;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);

    if (status != kStatus_Success) {
        return false;
    }

    uint8_t status_reg;
    do {
        flexspi_read_status(&status_reg);
    } while (status_reg & 1);
    printf("read page to cache finished\r\n");

    return (status == kStatus_Success);
}

void flexspi_page_read(uint32_t address, uint8_t *data, size_t len) {
    if (!flexspi_page_read_cache(address)) {
        printf("Failed to read to cache\r\n");
        return;
    }

    flexspi_transfer_t xfer;
    xfer.deviceAddress = address;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Read;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_READ_FROM_CACHE;
    xfer.data = (uint32_t*)data;
    xfer.dataSize = len;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
    printf("read page finished\r\n");
}

void flexspi_reset() {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Command;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_RESET;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
    uint8_t status_reg;
    do {
        flexspi_read_status(&status_reg);
    } while (status_reg & 1);
}

static uint8_t dummy = 0;
void flexspi_unlock() {
    flexspi_transfer_t xfer;
    xfer.deviceAddress = 0;
    xfer.port = kFLEXSPI_PortA1;
    xfer.cmdType = kFLEXSPI_Write;
    xfer.SeqNumber = 1;
    xfer.seqIndex = LUT_UNLOCK;
    xfer.data = (uint32_t*)&dummy;
    xfer.dataSize = 1;
    status_t status = FLEXSPI_TransferBlocking(FLEXSPI1, &xfer);
    printf("%s status: %ld %d\r\n", __func__, status, kStatus_Success);
}

uint8_t data[] = { 0xAA, 0xBB, 0xCC, 0xDD };
uint8_t data_out[ARRAY_SIZE(data)] = { 0, 0, 0, 0 };
extern "C" void app_main(void *param) {
    printf("FlexSPI NAND test %s %s.\r\n", __DATE__, __TIME__);
    flexspi_init();
    printf("FlexSPI NAND test past flexspi init %s %s.\r\n", __DATE__, __TIME__);
    flexspi_reset();
    flexspi_unlock();
    uint8_t status;
    flexspi_read_status(&status);
    uint8_t lock_status;
    flexspi_read_lock_status(&lock_status);
    uint16_t vendor_id;
    flexspi_get_vendor_id(&vendor_id);
    printf("Vendor id: 0x%x\r\n", vendor_id);
    flexspi_erase_block(0);
    flexspi_page_write(0, data, sizeof(data));

    flexspi_page_read(0, data_out, sizeof(data_out));
    DCACHE_InvalidateByRange((uint32_t)&data_out[0], sizeof(data_out));
    printf("0x%x\r\n", data_out[2]);
    DCACHE_InvalidateByRange((uint32_t)0x30000000U, sizeof(data_out));
    printf("0x30000000: 0x%lx\r\n", *((uint32_t*)0x30000000U));
    while (true) {
        taskYIELD();
    }
}
