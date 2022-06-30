#include "libs/base/strings.h"

namespace coralmicro {

std::string StrToHex(const char* src, size_t src_len) {
    std::string output;
    output.reserve(src_len * 2);
    for (size_t dest_idx = 0, src_idx = 0; src_idx < src_len;
         dest_idx += 2, src_idx += 1) {
        sprintf(&output[dest_idx], "%02x", src[src_idx]);
    }
    return output;
}

std::string StrToHex(const std::string& src) {
    return StrToHex(src.data(), src.size());
}
}  // namespace coralmicro