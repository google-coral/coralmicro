#ifndef APPS_FLEXSPI_NAND_FLEXSPI_NAND_LUT_H_
#define APPS_FLEXSPI_NAND_FLEXSPI_NAND_LUT_H_

#include <stdint.h>

extern const uint32_t customLUT[64];

#define LUT_READ_FROM_CACHE (0)
#define LUT_READ_STATUS     (1)
#define LUT_READID          (2)
#define LUT_WRITE_ENABLE    (3)
#define LUT_RESET           (4)
#define LUT_BLOCK_ERASE     (5)
#define LUT_UNLOCK          (6)
#define LUT_READ_LOCK_STATUS (7)
#define LUT_PROGRAM_LOAD    (9)
#define LUT_READ_PAGE       (11)
#define LUT_READ_ECC_STATUS (13)
#define LUT_PROGRAM_EXECUTE (14)

#endif  // APPS_FLEXSPI_NAND_FLEXSPI_NAND_LUT_H_
