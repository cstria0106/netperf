#pragma once

#include "conn.h"
#include "fd.h"
#include "options.h"
#include "server.h"
#include <linux/ip.h>
#include <netinet/in.h>
#include <memory>

class UdpConn : public Conn {
  Fd fd_;
  sockaddr_in destination_;
  bool finished_ = false;

 public:
  explicit UdpConn(sockaddr_in destination, Plan const& plan);
  int Send(char const* data, int size) override;
  int Receive(char data[], int size, int& skip_hint) override;
  int AdditionalBufferSize() override;

  static std::shared_ptr<UdpConn> Create(sockaddr_in destination,
                                         Plan const& plan);
  void Shutdown() override;
};