#pragma once

#include "error.h"
#include "fd.h"
#include "fmt/format.h"
#include "test.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdint>
#include <variant>

class Line {
  Fd fd_;
  sockaddr_in address_;

  void WriteLong(uint32_t value) const {
    auto result = htonl(value);
    if (send(fd_.Value(), &result, sizeof(result), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to send");
    }
  }

  void WriteShort(uint16_t value) const {
    auto result = htons(value);
    if (send(fd_.Value(), &result, sizeof(result), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to send");
    }
  }

  uint32_t ReadLong() const {
    uint32_t result;
    if (recv(fd_.Value(), &result, sizeof(result), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to recv");
    }
    return ntohl(result);
  }

  uint16_t ReadShort() const {
    uint16_t result;
    if (recv(fd_.Value(), &result, sizeof(result), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to recv");
    }
    return ntohs(result);
  }

  uint8_t ReadByte() const {
    uint8_t result;
    if (recv(fd_.Value(), &result, sizeof(result), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to recv");
    }
    return result;
  }

  void WriteByte(uint8_t value) const {
    if (send(fd_.Value(), &value, sizeof(value), MSG_NOSIGNAL) < 0) {
      throw StandardError("failed to send");
    }
  }

  Plan ReadPlan() {
    Plan plan;
    plan.time = ReadLong();
    plan.block_len = ReadLong();
    plan.bandwidth = ReadLong();
    plan.protocol = static_cast<Protocol>(ReadShort());
    plan.server_sends = ReadByte() != 0;
    plan.window_size = ReadLong();
    plan.no_delay = ReadByte() != 0;
    plan.zero_copy = ReadByte() != 0;
    return plan;
  }

  void WritePlan(Plan const& plan) {
    WriteLong(plan.time);
    WriteLong(plan.block_len);
    WriteLong(plan.bandwidth);
    WriteShort(static_cast<uint16_t>(plan.protocol));
    WriteByte(plan.server_sends ? 1 : 0);
    WriteLong(plan.window_size);
    WriteByte(plan.no_delay ? 1 : 0);
    WriteByte(plan.zero_copy ? 1 : 0);
  }

 public:
  class Signal {
   public:
    enum class Type {
      kStartTest = 1,
      kConfirmPlan = 2,
      kStopTest = 3,
    };

    struct StartTest {
      Plan plan;
      explicit StartTest(Plan plan) : plan(plan) {}
    };

    struct ConfirmPlan {
      int id;
      Plan plan;
      ConfirmPlan(int id, Plan plan) : id(id), plan(plan) {}
    };

    struct StopTest {};
  };

  explicit Line(Fd const& fd, sockaddr_in address) : fd_(std::move(fd)) {
    address_ = address;
  }

 private:
  void WriteSignalType(Signal::Type type) {
    WriteByte(static_cast<uint8_t>(type));
  }

  Signal::Type ReadSignalType() {
    uint8_t type = ReadByte();
    return static_cast<Signal::Type>(type);
  }

  void WaitForSignal(Signal::Type type) {
    while (true) {
      auto signal = ReadSignalType();
      if (signal == type) {
        return;
      }
    }
  }

 public:
  Signal::StartTest ReadStartTest() {
    WaitForSignal(Signal::Type::kStartTest);
    auto plan = ReadPlan();
    return Signal::StartTest(plan);
  }

  Signal::ConfirmPlan ReadConfirmPlan() {
    WaitForSignal(Signal::Type::kConfirmPlan);
    int id = ReadLong();
    auto plan = ReadPlan();
    return Signal::ConfirmPlan(id, plan);
  }

  Signal::StopTest ReadStopTest() {
    WaitForSignal(Signal::Type::kStopTest);
    return Signal::StopTest();
  }

  void WriteStartTest(Plan const& plan) {
    WriteSignalType(Signal::Type::kStartTest);
    WritePlan(plan);
  }

  void WriteConfirmPlan(int id, Plan const& plan) {
    WriteSignalType(Signal::Type::kConfirmPlan);
    WriteLong(id);
    WritePlan(plan);
  }

  void WriteStopTest() { WriteSignalType(Signal::Type::kStopTest); }

  sockaddr_in Address() const { return address_; }

  Fd Fd() const { return fd_; }
};
