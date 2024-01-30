#include "udp.h"
#include "fmt/base.h"
#include "fmt/format.h"
#include "raw_socket.h"
#include "values.h"
#include <asm-generic/socket.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <stdexcept>

UdpConn::UdpConn(sockaddr_in destination, Plan const& plan)
    : destination_(destination) {
  int raw_fd;
  raw_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (raw_fd < 0) {
    throw StandardError("failed to create raw socket");
  };

  fd_ = Fd(raw_fd);
  fd_.SetSocketOptions(plan);

  sockaddr_in address;
  address.sin_port = htons(kUdpPort);
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(fd_.Value(), reinterpret_cast<sockaddr*>(&address),
           sizeof(address)) < 0) {
    throw StandardError("failed to bind");
  }
}

int UdpConn::Send(char const* data, int size) {
  int sent;
  struct sockaddr addr;
  sent =
      sendto(fd_.Value(), data, size, MSG_NOSIGNAL,
             reinterpret_cast<sockaddr*>(&destination_), sizeof(destination_));
  if (sent < 0) {
    if (errno == ENOBUFS) {
      return 0;
    }
    throw StandardError("failed to send");
  }
  return sent;
}

int UdpConn::Receive(char data[], int size, int& skip_hint) {
  auto received = recv(fd_.Value(), data, size, MSG_NOSIGNAL);
  if (received < 0) {
    fmt::println("failed to receive: {}", strerror(errno));
  }
  if (received == 0) return -1;
  skip_hint = 0;
  return received;
}

int UdpConn::AdditionalBufferSize() { return 0; }

std::shared_ptr<UdpConn> UdpConn::Create(sockaddr_in destination,
                                         Plan const& plan) {
  destination.sin_port = htons(kUdpPort);
  return std::make_shared<UdpConn>(destination, plan);
}
void UdpConn::Shutdown() { shutdown(fd_.Value(), SHUT_RDWR); }
