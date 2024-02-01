#include "receive.h"
#include "fmt/base.h"
#include <vector>

void Receiver::Start(Conn& conn, Test& test) {
  std::vector<char> buffer(test.plan.block_len + conn.AdditionalBufferSize(), 0);
  while (true) {
    int skip_hint;
    int received = conn.Receive(buffer.data(), buffer.size(), skip_hint);
    if (received < 0) break;
    test.rx += received - skip_hint;
    test.rx_packets++;
  }
}