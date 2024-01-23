#pragma once

#include "line.h"
#include "options.h"
#include "test.h"
#include "values.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdexcept>

class LineClient {
 public:
  static Line Connect(char* ip);
};