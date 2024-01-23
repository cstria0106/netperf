#include "tcp.h"
#include "fmt/base.h"
#include "values.h"
#include <asm-generic/socket.h>
#include <linux/ip.h>
#include <sys/socket.h>
#include <cstdio>
#include <memory>
#include <stdexcept>

std::pair<Fd, sockaddr_in> Tcp::Accept(Fd const& fd) {
  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);
  int raw_fd = accept(fd.Value(), reinterpret_cast<struct sockaddr*>(&address),
                      &addrlen);
  if (raw_fd < 0) {
    throw StandardError("failed to accept");
  }
  return std::make_pair(Fd(raw_fd), address);
}

std::pair<Fd, sockaddr_in> Tcp::Connect(char* ip, uint16_t port) {
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);
  return Connect(address);
}

std::pair<Fd, sockaddr_in> Tcp::Connect(sockaddr_in address) {
  int raw_fd;
  raw_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (raw_fd < 0) {
    throw StandardError("failed to create tcp socket");
  }

  auto fd = Fd(raw_fd);

  int try_count = 0;
  while (connect(fd.Value(), reinterpret_cast<struct sockaddr*>(&address),
                 sizeof(address)) < 0) {
    if (try_count++ > 10) {
      throw StandardError("failed to connect");
    }
    sleep(1);
  }

  return std::make_pair(fd, address);
}

Fd Tcp::Listen(int port) {
  int raw_fd;
  raw_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (raw_fd < 0) {
    throw StandardError("failed to create tcp socket");
  }
  auto fd = Fd(raw_fd);

  // set SO_REUSEADDR
  int optval = 1;
  if (setsockopt(fd.Value(), SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) < 0) {
    throw StandardError("failed to set SO_REUSEADDR");
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(fd.Value(), reinterpret_cast<struct sockaddr*>(&addr),
           sizeof(addr)) < 0) {
    throw StandardError("failed to bind");
  }

  if (listen(fd.Value(), SOMAXCONN) < 0) {
    throw StandardError("failed to listen");
  }

  return fd;
}

int TcpConn::Send(char const* data, int size) {
  int sent;
  if ((sent = send(fd_.Value(), data, size, MSG_NOSIGNAL)) < 0) {
    throw StandardError("failed to send");
  }
  return sent;
}

int TcpConn::Receive(char data[], int size, int& skip_hint) {
  int received;
  if ((received = recv(fd_.Value(), data, size, MSG_NOSIGNAL)) < 0) {
    fmt::println("failed to receive: {}", strerror(errno));
    return -1;
  }
  if (received == 0) return -1;
  skip_hint = 0;
  return received;
}

int TcpConn::AdditionalBufferSize() { return 0; }

void TcpConn::Shutdown() { shutdown(fd_.Value(), SHUT_RDWR); }

std::shared_ptr<TcpConn> TcpConn::CreateClientSide(Line const& line) {
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr = line.Address().sin_addr;
  address.sin_port = htons(kTcpPort);
  auto fd = Tcp::Connect(address);
  return std::make_shared<TcpConn>(fd.first);
}

std::shared_ptr<TcpConn> TcpConn::CreateServerSide(Line const& line) {
  auto fd = Tcp::Listen(kTcpPort);
  auto client_fd = Tcp::Accept(fd);
  return std::make_shared<TcpConn>(client_fd.first);
}
