#include "apps/HelloWorld/board.h"
#include "apps/HelloWorld/peripherals.h"
#include "apps/HelloWorld/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.h"

int main(int argc, char** argv) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();
    PRINTF("Hello world.\r\n");
    while (true);
    return 0;
}
