#pragma once

#include "conn.h"
#include "options.h"
#include <memory>

class Receiver {
 public:
  void Start(Conn& conn, Test& test);
};