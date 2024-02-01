#include "raw_socket.h"
#include "error.h"
#include "fmt/base.h"
#include "fmt/format.h"
#include "test.h"
#include "values.h"
#include <asm-generic/socket.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <stdexcept>

RawSocketConn::RawSocketConn(sockaddr_in destination, char const* interface,
                             Plan const& plan)
    : destination_(destination) {
  int raw_fd = socket(AF_INET, SOCK_RAW, kRawProto);
  if (raw_fd < 0) {
    throw StandardError("failed to create raw socket");
  };
  fd_ = Fd(raw_fd);
  fd_.SetSocketOptions(plan);

  if (setsockopt(fd_.Value(), SOL_SOCKET, SO_BINDTODEVICE, interface,
                 sizeof(interface)) < 0) {
    throw StandardError("failed to bind to device");
  }
}

int RawSocketConn::Send(char const* data, int size) {
  int sent;
  struct sockaddr addr;
  if ((sent = sendto(fd_.Value(), data, size, MSG_NOSIGNAL,
                     reinterpret_cast<sockaddr*>(&destination_),
                     sizeof(destination_))) < 0) {
    if (errno == ENOBUFS) {
      return 0;
    }
    throw StandardError("failed to send");
  }
  return sent;
}

int RawSocketConn::Receive(char data[], int size, int& skip_hint) {
  uint8_t header[sizeof(iphdr)];
  iovec iov[2];
  msghdr msg;
  memset(&msg, 0, sizeof(msg));
  memset(iov, 0, sizeof(iov));
  iov[0].iov_base = header;
  iov[0].iov_len = sizeof(header);
  iov[1].iov_base = data;
  iov[1].iov_len = size;
  msg.msg_iov = iov;
  msg.msg_iovlen = sizeof(iov) / sizeof(iov[0]);

  int received = recvmsg(fd_.Value(), &msg, MSG_NOSIGNAL);
  if (received < 0) {
    fmt::println("failed to receive: {}", strerror(errno));
  }
  if (received == 0) return -1;
  skip_hint = 0;
  return received - sizeof(header);
}

int RawSocketConn::AdditionalBufferSize() { return 0; }

std::shared_ptr<RawSocketConn> RawSocketConn::Create(sockaddr_in address,
                                                     char const* interface,
                                                     Plan const& plan) {
  return std::make_shared<RawSocketConn>(address, interface, plan);
}
void RawSocketConn::Shutdown() { shutdown(fd_.Value(), SHUT_RDWR); }
