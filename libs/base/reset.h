#ifndef LIBS_BASE_RESET_H_
#define LIBS_BASE_RESET_H_

#include <cstdint>

namespace coral::micro {

struct ResetStats {
    uint32_t reset_reason;
    uint32_t watchdog_resets;
    uint32_t lockup_resets;
};

void ResetToBootloader();
void ResetToFlash();
void StoreResetReason();
ResetStats GetResetStats();

}  // namespace coral::micro

#endif  // LIBS_BASE_RESET_H_
