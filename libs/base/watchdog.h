#ifndef LIBS_BASE_WATCHDOG_H_
#define LIBS_BASE_WATCHDOG_H_

namespace coral::micro {
namespace watchdog {

struct WatchdogConfig {
    int timeout_s;
    int pet_rate_s;
    bool enable_irq;
    int irq_s_before_timeout;
};

// If enabling the interrupt, be sure to extern WDOG1_IRQHandler. Otherwise
// you will call DefaultISR.
void StartWatchdog(const WatchdogConfig& config);
void StopWatchdog();

}  // namespace watchdog
}  // namespace coral::micro

#endif  // LIBS_BASE_WATCHDOG_H_
