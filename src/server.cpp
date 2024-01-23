

#include "server.h"
#include "fmt/core.h"
#include "tcp.h"
#include "values.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

LineServer LineServer::Listen() {
  auto fd = Tcp::Listen(kLinePort);
  return LineServer(std::move(fd));
}

Line LineServer::Accept() const {
  auto [fd, address] = Tcp::Accept(fd_);
  return Line(fd, address);
}

int LineServer::NextTestId() {
  test_id_mutex_.lock();
  int result = test_id_;
  test_id_++;
  test_id_mutex_.unlock();
  return result;
}
