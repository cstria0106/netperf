#include "client.h"
#include "tcp.h"

Line LineClient::Connect(char* ip) {
  auto [fd, address] = Tcp::Connect(ip, kLinePort);
  return Line(fd, address);
}
