#pragma once

#include "fmt/base.h"
#include <memory>
#include <unistd.h>

class FdInner {
  friend class Fd;
  int fd_;
  int Value() const { return fd_; }

 public:
  explicit FdInner(int fd) : fd_(fd) {}
  ~FdInner() { close(fd_); }
};

class Fd {
  std::shared_ptr<FdInner> fd_;

 public:
  Fd() = default;
  explicit Fd(int fd) : fd_(std::make_shared<FdInner>(fd)) {}
  int Value() const { return fd_->Value(); }
};