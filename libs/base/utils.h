#ifndef LIBS_BASE_UTILS_H_
#define LIBS_BASE_UTILS_H_

#include <cstdint>
#include <string>

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/ip_addr.h"

#define FUSE_ADDRESS_TO_OCOTP_INDEX(fuse) ((fuse - 0x800) >> 4)
#define MAC1_ADDR_LO (0xA80)
#define MAC1_ADDR_HI (0xA90)

namespace coral::micro {

// Represents the six bytes of MAC address.
struct MacAddress {
    MacAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
        : a(a), b(b), c(c), d(d), e(e), f(f) {}
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
};

namespace utils {

// Gets the 64-bit unique identifiers of the RT1176.
// @returns 64-bit value that is unique to the device.
uint64_t GetUniqueID();

// Gets the hex string representation of the unique identifier.
// @returns String containing the 64-bit unique identifier, as a printable hex string.
std::string GetSerialNumber();

// Gets the assigned MAC address from the device fuses.
// @returns The MAC address assigned to the device.
MacAddress GetMacAddress();

// Gets the USB IP address that is stored in flash memory.
// @param usb_ip_out The `ip4_addr_t` structure in which to store the IP.
// @returns True if the address was successfully retrieved; false otherwise.
bool GetUSBIPAddress(ip4_addr_t* usb_ip_out);

// Gets the USB IP address that is stored in flash memory.
// @param usb_ip_out A pointer to a string in which to store a printable version of the IP.
// @returns True if the address was successfully retrieved; false otherwise.
bool GetUSBIPAddress(std::string* usb_ip_out);

// Gets the Wi-Fi SSID that is stored in flash memory.
// @param wifi_ssid_out A pointer to a string in which to store the SSID.
// @returns True if the SSID was successfully retrieved; false otherwise.
bool GetWiFiSSID(std::string* wifi_ssid_out);

// Sets the Wi-Fi SSID in flash memory.
// @param wifi_ssid A pointer to a string containing the SSID.
// @returns True if the SSID was successfully stored; false otherwise.
bool SetWiFiSSID(std::string* wifi_ssid);

// Gets the Wi-Fi key that is stored in flash memory.
// @param wifi_ssid_out A pointer to a string in which to store the SSID.
// @returns True if the SSID was successfully retrieved; false otherwise.
bool GetWiFiPSK(std::string* wifi_psk_out);

// Sets the Wi-Fi key in flash memory.
// @param wifi_psk A pointer to a string containing the key.
// @returns True if the key was successfully stored; false otherwise.
bool SetWiFiPSK(std::string* wifi_psk);

// Retrieves the ethernet speed that is stored in flash memory.
// @returns The ethernet speed in Mbps.
//   The default return value is 100, if no value is stored in flash.
int GetEthernetSpeed();

}  // namespace utils
}  // namespace coral::micro

#endif  // LIBS_BASE_UTILS_H_
