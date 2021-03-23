#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/wwd_assert.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/resources.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_resource.h"
#include <stdio.h>

#define PAGE_SIZE (2048)
#define BLOCK_PAGE_COUNT (64)

extern nand_handle_t* BOARD_GetNANDHandle(void);

resource_result_t platform_read_external_resource(const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize, uint32_t* size, void* buffer) {
    nand_handle_t* nand_handle = BOARD_GetNANDHandle();
    wiced_assert("NAND uninitialized!\r\n", nand_handle);
    uint32_t resource_base_page = 0;
    if (resource == &wifi_firmware_image) {
        resource_base_page = 0xD * BLOCK_PAGE_COUNT;
    } else if (resource == &wifi_firmware_clm_blob) {
        resource_base_page = 0x12 * BLOCK_PAGE_COUNT;
    } else {
        wiced_assert("Unknown resource!", false);
    }
    uint32_t resource_page = offset / PAGE_SIZE;
    uint32_t page_offset = offset % PAGE_SIZE;
    uint32_t size_to_read = MIN((PAGE_SIZE - page_offset), maxsize);
    status_t status = Nand_Flash_Read_Page_Partial(nand_handle, resource_base_page + resource_page, page_offset, buffer, size_to_read);
    if (status != kStatus_Success) {
        return RESOURCE_FILE_READ_FAIL;
    }
    if (size_to_read < maxsize) {
        status_t status = Nand_Flash_Read_Page_Partial(nand_handle, resource_base_page + resource_page + 1, 0, buffer + size_to_read, maxsize - size_to_read);
        if (status != kStatus_Success) {
            return RESOURCE_FILE_READ_FAIL;
        }
    }
    *size = maxsize;
    return RESOURCE_SUCCESS;
}

int platform_wprint_permission(void) {
    return 1;
}
