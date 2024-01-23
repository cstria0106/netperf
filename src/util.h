#pragma once

#include <sys/time.h>
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
  return std::chrono::duration_cast<Seconds>(duration).count();
}

inline Time Now() { return std::chrono::system_clock::now(); }

inline std::string HumanSize(uint64_t bytes) {
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
