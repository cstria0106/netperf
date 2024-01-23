#include "send.h"
#include "fmt/base.h"
#include "options.h"
#include "util.h"
#include <thread>
#include <vector>

void Sender::Start(Conn& conn, Test& test) {
  std::vector<char> buffer(test.plan.block_len, 0);

  auto start_time = Now();
  while (true) {
    auto now = Now() - start_time;

    while (test.plan.bandwidth <= 0 ||
           ToSeconds(now) * test.plan.bandwidth > tx_bits_) {
      int sent = conn.Send(buffer.data(), buffer.size());
      test.tx += sent;
      test.tx_packets++;
      tx_bits_ += sent * 8;
      if (test.Finished()) {
        break;
      }
    }

    if (test.Finished()) {
      break;
    }
  }
}