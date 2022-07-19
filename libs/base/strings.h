/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_BASE_STRINGS_H_
#define LIBS_BASE_STRINGS_H_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace coralmicro {

// Gets the length of a const char.
//
// This function is preferred over `strlen()` for constant strings that can be
// evaluated at compile time.
// @param str The string to get length.
// @tparam N The string array length determined by the compiler through function
// template argument deduction.
// @return The length of the const char string.
template <size_t N>
constexpr size_t StrLen(const char (&str)[N]) {
  return N - 1;
}

// Checks if a string starts with a prefix.
//
// @param s The full string to check for prefix.
// @param prefix The prefix to look for at the beginning of the string s.
// @tparam N The length of the string.
// @return True if prefix is the prefix of s, else false.
template <size_t N>
bool StrStartsWith(const char* s, const char (&prefix)[N]) {
  return std::strncmp(s, prefix, StrLen(prefix)) == 0;
}

// Checks if a string ends with a suffix.
//
// @param s The full string to check for suffix.
// @param suffix The suffix to look for at the end of the string s.
// @tparam N The length of the string.
// @return True if suffix is the suffix of s, else false.
template <size_t N>
bool StrEndsWith(const std::string& s, const char (&suffix)[N]) {
  if (s.size() < StrLen(suffix)) return false;
  return std::strcmp(s.c_str() + s.size() - StrLen(suffix), suffix) == 0;
}

// Appends a string to a source string.
//
// @param v The source string to append format_str string to.
// @param format_str The string with format specifier to append to v.
// @param args The arguments to the format specifier.
// @tparam T The variadic class template that allows args to be any type.
template <typename C, typename... T>
void StrAppend(C* v, const char* format_str, T... args) {
  static_assert(sizeof((*v)[0]) == 1);

  const int size = std::snprintf(nullptr, 0, format_str, args...) + 1;
  v->resize(v->size() + size);
  auto* s = reinterpret_cast<char*>(v->data() + v->size() - size);
  std::snprintf(s, size, format_str, args...);
  v->pop_back();  // remove null terminator
}

// Returns a string's hexadecimal representation.
//
// @param s The source array of raw characters.
// @param size The size of the source array.
// @return The hexadecimal representation of the string.
std::string StrToHex(const char* s, size_t size);

// Returns a string's hexadecimal representation.
//
// @param s The source string.
// @return The hexadecimal representation of the string.
inline std::string StrToHex(const std::string& s) {
  return StrToHex(s.data(), s.size());
}

}  // namespace coralmicro

#endif  // LIBS_BASE_STRINGS_H_
