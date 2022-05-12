#include "libs/nxp/rt1176-sdk/board_hardware.h"

extern "C" void app_main(void* param);

extern "C" int main(int argc, char** argv) __attribute__((weak));
extern "C" int main(int argc, char** argv) {
    BOARD_InitHardware(true);

    app_main(nullptr);

    return 0;
}
