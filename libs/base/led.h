#ifndef LIBS_BASE_LED_H_
#define LIBS_BASE_LED_H_

namespace coral::micro {
namespace led {

enum class LED {
    kPower,
    kUser,
    kTpu,
};

inline constexpr unsigned int kFullyOn = 100;
inline constexpr unsigned int kFullyOff = 0;

bool Set(LED led, bool enable);
bool Set(LED led, bool enable, unsigned int brightness);

}  // namespace led
}  // namespace coral::micro

#endif  // LIBS_BASE_LED_H_
