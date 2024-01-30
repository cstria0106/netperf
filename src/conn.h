#pragma once

#include "fd.h"
class Conn {
 public:
  virtual int Send(char const* data, int size) = 0;
  virtual int Receive(char data[], int size, int& skip_hint) = 0;
  virtual int AdditionalBufferSize() = 0;
  virtual ~Conn() = default;

  virtual void Shutdown() = 0;
};