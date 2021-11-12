#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/resources.h"

const resource_hnd_t wifi_firmware_image = {
    .location = RESOURCE_IN_EXTERNAL_STORAGE,
    .size = 580041,
    .val =
        {
            .fs =
                {
                    .filename =
                        "/third_party/firmware/cypress/43455C0-mfgtest.bin",
                    .offset = 0,
                },
        },
};

const resource_hnd_t wifi_firmware_clm_blob = {
    .location = RESOURCE_IN_EXTERNAL_STORAGE,
    .size = 5290,
    .val =
        {
            .fs =
                {
                    .filename = "/third_party/firmware/cypress/"
                                "43455C0-mfgtest.clm_blob",
                    .offset = 0,
                },
        },
};
