#ifndef __LIBS_BASE_STRINGS_H__
#define __LIBS_BASE_STRINGS_H__

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace valiant {

template <size_t N>
constexpr size_t StrLen(const char (&)[N]) {
    return N - 1;
}

template <size_t N>
bool StrStartsWith(const char* s, const char (&prefix)[N]) {
    return std::strncmp(s, prefix, StrLen(prefix)) == 0;
}

template <size_t N>
bool StrEndsWith(const std::string& s, const char (&suffix)[N]) {
    if (s.size() < StrLen(suffix)) return false;
    return std::strcmp(s.c_str() + s.size() - StrLen(suffix), suffix) == 0;
}

template <typename... T>
void StrAppend(std::vector<uint8_t>* v, const char* format, T... args) {
    const int size = std::snprintf(nullptr, 0, format, args...) + 1;
    v->resize(v->size() + size);
    auto* s = reinterpret_cast<char*>(v->data() + v->size() - size);
    std::snprintf(s, size, format, args...);
    v->pop_back();  // remove null terminator
}

}  // namespace valiant

#endif  // __LIBS_BASE_STRINGS_H__
