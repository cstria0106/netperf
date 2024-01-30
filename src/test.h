#pragma once

#include "fmt/core.h"
#include "util.h"
#include <sys/time.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <unistd.h>

enum class Protocol {
  kTCP = 0,
  kUDP = 1,
  kRawSocket = 2,
};

struct Plan {
  int time;
  int block_len;
  int bandwidth;
  bool server_sends;
  Protocol protocol;
  int window_size;
  bool no_delay;
  bool zero_copy;
};

struct Test {
 private:
  Time last_report_time_;
  uint64_t last_rx_;
  uint64_t last_tx_;

 public:
  int id;
  Time start_time;
  Plan plan;
  bool manually_stopped = false;

  bool started;
  uint64_t rx;
  uint64_t tx;
  uint64_t rx_packets;
  uint64_t tx_packets;

  void Stop() { manually_stopped = true; }

  Duration Duration() const { return Now() - start_time; }
  bool Finished() const {
    return manually_stopped || Duration() >= Seconds(plan.time);
  }

  double RxBandwidth() const {
    return static_cast<double>(rx - last_rx_) * 8 /
           ToSeconds(Now() - last_report_time_);
  }

  double TxBandwidth() const {
    return static_cast<double>(tx - last_tx_) * 8 /
           ToSeconds(Now() - last_report_time_);
  }

  void Report(bool finish = false) {
    if (finish) {
      fmt::println("time: {}s", ToSeconds(Duration()));
      fmt::println("rx: {}Bytes", FormatBytes(rx));
      fmt::println("tx: {}Bytes", FormatBytes(tx));
      fmt::println("tx packets: {}, rx packets: {}", tx_packets, rx_packets);
      fmt::println("");
    } else {
      fmt::println("time: {}s", ToSeconds(Duration()));
      fmt::println("rx: {}Bytes ({}bits/sec)", FormatBytes(rx),
                   FormatBytes(RxBandwidth()));
      fmt::println("tx: {}Bytes ({}bits/sec)", FormatBytes(tx),
                   FormatBytes(TxBandwidth()));
      fmt::println("tx packets: {}, rx packets: {}", tx_packets, rx_packets);
      fmt::println("");
      last_report_time_ = Now();
      last_tx_ = tx;
      last_rx_ = rx;
    }
  }

  explicit Test(int id, Plan plan) {
    this->id = id;
    this->plan = plan;
    this->started = false;
    this->rx = 0;
    this->tx = 0;
    this->rx_packets = 0;
    this->tx_packets = 0;
    this->last_rx_ = 0;
    this->last_tx_ = 0;
  }

  void Initialize() {
    start_time = Now();
    last_report_time_ = start_time;
    started = true;
  }
};