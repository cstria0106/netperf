#include "client.h"
#include "conn.h"
#include "fmt/base.h"
#include "fmt/format.h"
#include "line.h"
#include "options.h"
#include "raw_socket.h"
#include "receive.h"
#include "report.h"
#include "send.h"
#include "server.h"
#include "tcp.h"
#include "test.h"
#include <bits/getopt_core.h>
#include <fmt/core.h>
#include <condition_variable>
#include <cstdio>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unistd.h>

std::shared_ptr<Conn> CreateTransferConnection(Line& line, Plan const& plan,
                                               Options const& options) {
  switch (plan.protocol) {
    case Protocol::kTCP:
      if (options.server) return TcpConn::CreateServerSide(line);
      if (options.client) return TcpConn::CreateClientSide(line);

    case Protocol::kRawSocket:
      return RawSocketConn::Create(line.Address(), options.interface);
  }
}

void StartTest(Test& test, Conn& connection, Options const& options) {
  test.Initialize();

  StartReportThread(test, 1.0);

  fmt::println("test {} started", test.id);

  if ((options.server && test.plan.server_sends) ||
      (options.client && !test.plan.server_sends)) {
    Sender sender;
    sender.Start(connection, test);
  } else {
    Receiver receiver;
    receiver.Start(connection, test);
  }

  fmt::println("test {} finished", test.id);
}

void StartServer(Options options) {
  auto server = LineServer::Listen();
  while (true) {
    auto line = server.Accept();
    fmt::println("new connection: {}", line.Address().sin_port);

    std::thread(
        [&server](Options options, Line line) {
          try {
            auto start_test = line.ReadStartTest();

            auto test = Test(server.NextTestId(), start_test.plan);
            line.WriteConfirmPlan(test.id, test.plan);

            auto connection =
                CreateTransferConnection(line, test.plan, options);

            StartTest(test, *connection, options);
            line.WriteStopTest();
          } catch (std::exception& e) {
            fmt::println("error: {}", e.what());
          }
        },
        options, std::move(line))
        .detach();
  }
}

void StartClient(Options const& options) {
  auto line = LineClient::Connect(options.address);

  // Build plan and start test
  Plan plan;
  plan.protocol = options.protocol;
  plan.time = options.time;
  plan.block_len = options.block_len;
  plan.bandwidth = options.bandwidth;
  plan.server_sends = options.client && options.receiver;
  line.WriteStartTest(plan);

  // Wait for confirmed plan
  auto confirm_plan = line.ReadConfirmPlan();

  // Create connection for transfer
  auto connection = CreateTransferConnection(line, confirm_plan.plan, options);

  // Run waiting thread for test end
  std::thread([connection, &line]() {
    line.ReadStopTest();
    connection->Shutdown();
  }).detach();

  // Start test
  Test test(confirm_plan.id, confirm_plan.plan);
  StartTest(test, *connection, options);
}

void Program(int argc, char** argv) {
  Options options;
  options = ParseOptions(argc, argv);

  if (options.server && options.client) {
    throw std::runtime_error(
        "server and client cannot be specified at the same time");
  }

  if (!(options.server || options.client)) {
    throw std::runtime_error("one of server or client must be specified");
  }

  if (options.server) {
    StartServer(options);
  }

  if (options.client) {
    StartClient(options);
  }
}

int main(int argc, char** argv) {
  try {
    Program(argc, argv);
  } catch (std::exception& e) {
    fprintf(stderr, "%s\n", e.what());
    return 1;
  }
}