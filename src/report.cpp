#include "report.h"
#include "fmt/base.h"
#include "util.h"
#include <chrono>
#include <cstdint>
#include <ratio>

void StartReportThread(Test& test, double interval) {
  std::thread([&test, interval]() {
    while (true) {
      std::this_thread::sleep_for(
          Microseconds(static_cast<uint64_t>(interval * 1000000)));

      if (test.Finished()) {
        break;
      }

      if (!test.started) continue;
      test.Report();
    }
  }).detach();
}