#include "libs/nxp/rt1176-sdk/board.h"
#include "libs/nxp/rt1176-sdk/peripherals.h"
#include "libs/nxp/rt1176-sdk/pin_mux.h"

void BOARD_InitHardware(void) {
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();
  BOARD_InitDebugConsole();
}
