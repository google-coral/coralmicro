#ifndef _LIBS_BASE_LED_H_
#define _LIBS_BASE_LED_H_

namespace coral::micro {
namespace led {

enum class LED {
    kPower,
    kUser,
    kTpu,
};

constexpr const unsigned int kFullyOn = 100;
constexpr const unsigned int kFullyOff = 0;

bool Set(LED led, bool enable);
bool Set(LED led, bool enable, unsigned int brightness);

}  // namespace led
}  // namespace coral::micro

#endif  // _LIBS_BASE_LED_H_
