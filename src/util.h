#pragma once

#include "fmt/format.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

using Time = std::chrono::system_clock::time_point;
using Duration = std::chrono::duration<double>;
using Seconds = std::chrono::seconds;
using Microseconds = std::chrono::microseconds;
using Miliseconds = std::chrono::milliseconds;
inline double ToSeconds(Duration const& duration) {
  return std::chrono::duration_cast<Microseconds>(duration).count() / 1000000.0;
}

inline Time Now() { return std::chrono::system_clock::now(); }

inline uint64_t ParseBytes(std::string const& str) {
  uint64_t unit_size = 1;
  // check string is number except last character
  auto not_numeric = str.find_first_not_of("0123456789");
  if (not_numeric != str.size() - 1 && not_numeric != std::string::npos) {
    throw std::runtime_error(fmt::format("invalid unit: {}", str));
  }

  if (str.ends_with("K") || str.ends_with("k")) {
    unit_size = 1000;
  } else if (str.ends_with("M") || str.ends_with("m")) {
    unit_size = 1000 * 1000;
  } else if (str.ends_with("G") || str.ends_with("g")) {
    unit_size = 1000 * 1000 * 1000;
  } else if (not_numeric == std::string::npos) {
    return std::stoull(str);
  } else {
    throw std::runtime_error(fmt::format("invalid unit: {}", str));
  }

  return std::stoull(str.substr(0, str.size() - 1)) * unit_size;
}

inline std::string FormatBytes(uint64_t bytes) {
  char const* suffix[] = {"", "K", "M", "G", "T"};
  char length = sizeof(suffix) / sizeof(suffix[0]);

  int i = 0;
  double double_bytes = bytes;

  if (bytes > 1000) {
    for (i = 0; (bytes / 1000) > 0 && i < length - 1; i++, bytes /= 1000)
      double_bytes = bytes / 1000.0;
  }

  char output[64];
  sprintf(output, "%.02lf %s", double_bytes, suffix[i]);
  return std::string(output);
}
