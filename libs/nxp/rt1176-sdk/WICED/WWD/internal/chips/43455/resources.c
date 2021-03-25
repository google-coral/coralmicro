#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/resources.h"

const resource_hnd_t wifi_firmware_image = {
    .location = RESOURCE_IN_EXTERNAL_STORAGE,
    .size = 613808,
    .val = {
        .fs = {
            .filename = "/firmware/43455C0.bin",
            .offset = 0,
        },
    },
};

const resource_hnd_t wifi_firmware_clm_blob = {
    .location = RESOURCE_IN_EXTERNAL_STORAGE,
    .size = 14828,
    .val = {
        .fs = {
            .filename = "/firmware/43455C0.clm_blob",
            .offset = 0,
        },
    },
};

