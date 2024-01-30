#pragma once

#include "error.h"
#include "fmt/base.h"
#include "options.h"
#include "test.h"
#include <asm-generic/socket.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
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

  void SetSocketOptions(Plan const& plan) {
    if (plan.window_size > 0) {
      fmt::println("setting window size to {}", plan.window_size);
      if (setsockopt(fd_->Value(), SOL_SOCKET, SO_RCVBUF, &plan.window_size,
                     sizeof(plan.window_size)) < 0) {
        throw StandardError("failed to set SO_RCVBUF");
      }
      if (setsockopt(fd_->Value(), SOL_SOCKET, SO_SNDBUF, &plan.window_size,
                     sizeof(plan.window_size)) < 0) {
        throw StandardError("failed to set SO_SNDBUF");
      }
    }

    if (plan.no_delay && plan.protocol == Protocol::kTCP) {
      int value = 1;
      if (setsockopt(fd_->Value(), IPPROTO_TCP, TCP_NODELAY, &value,
                     sizeof(value)) < 0) {
        throw StandardError("failed to set TCP_NODELAY");
      }
    }
  }
};