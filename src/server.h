#pragma once

#include "fd.h"
#include "line.h"
#include "test.h"
#include <netinet/in.h>
#include <mutex>
#include <stdexcept>
#include <unistd.h>

class LineServer {
  int test_id_ = 0;
  std::mutex test_id_mutex_;
  Fd fd_;

  explicit LineServer(Fd fd) : fd_(std::move(fd)) {}

 public:
  static LineServer Listen();
  Line Accept() const;

  int NextTestId();
};