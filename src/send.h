#pragma once

#include "conn.h"
#include "options.h"
#include "util.h"
#include <cstdint>
#include <memory>

class Sender {
  uint64_t tx_bits_ = 0;

 public:
  Sender() = default;
  void Start(Conn& conn, Test& test);
};