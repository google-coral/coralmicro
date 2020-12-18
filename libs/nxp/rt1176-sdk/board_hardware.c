#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/peripherals.h"
#include "libs/nxp/rt1176-sdk/pin_mux.h"
#include "third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.h"

void SystemInitHook(void) {
    MCMGR_EarlyInit();
}

void BOARD_InitHardware(void) {
    MCMGR_Init();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#if __CORTEX_M == 7
    BOARD_InitDebugConsole();
#endif
}
