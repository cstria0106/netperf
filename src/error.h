#pragma once

#include "fmt/format.h"
#include <cerrno>

class StandardError : public std::runtime_error {
 public:
  explicit StandardError(char const* message)
      : std::runtime_error(fmt::format("{}: {}", message, strerror(errno))) {}
};