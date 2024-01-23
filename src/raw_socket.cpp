#include "raw_socket.h"
#include "fmt/base.h"
#include "fmt/format.h"
#include "values.h"
#include <asm-generic/socket.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <stdexcept>

RawSocketConn::RawSocketConn(sockaddr_in destination, char* interface)
    : destination_(destination) {
  int fd;
  if ((fd = socket(AF_INET, SOCK_RAW, kRawProto)) < 0) {
    throw StandardError("failed to create raw socket");
  };
  fd_ = Fd(fd);

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

int n = 0;
int RawSocketConn::Receive(char data[], int size, int& skip_hint) {
  int received;
  n++;
  received = recv(fd_.Value(), data, size, MSG_NOSIGNAL);
  if (received < 0) {
    fmt::println("failed to receive: {}", strerror(errno));
  }
  if (received == 0) {
    return -1;
  }
  skip_hint = sizeof(iphdr);
  return received;
}

int RawSocketConn::AdditionalBufferSize() { return sizeof(iphdr); }

std::shared_ptr<RawSocketConn> RawSocketConn::Create(sockaddr_in address,
                                                     char* interface) {
  return std::make_shared<RawSocketConn>(address, interface);
}
void RawSocketConn::Shutdown() { shutdown(fd_.Value(), SHUT_RDWR); }
