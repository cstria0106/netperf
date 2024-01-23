#pragma once

#include "fmt/core.h"
#include "test.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>

struct Options {
  Protocol protocol;
  char* address;
  bool client;
  bool server;
  bool receiver;
  int time;
  int bandwidth;
  int block_len;
  char* interface;
};

inline Options ParseOptions(int argc, char** argv) {
  Options options;
  options.server = false;
  options.protocol = Protocol::kTCP;
  options.address = nullptr;
  options.server = false;
  options.receiver = false;
  options.time = 10;
  options.block_len = 1000;
  options.bandwidth = 0;

  opterr = 0;
  char option;
  while ((option = getopt(argc, argv, "sc:rt:l:b:i:p:")) != -1) {
    switch (option) {
      case 's':
        options.server = true;
        break;
      case 'c':
        options.client = true;
        options.address = optarg;
        break;
      case 'r':
        options.receiver = true;
        break;
      case 't':
        options.time = atoi(optarg);
        break;
      case 'l':
        options.block_len = atoi(optarg);
        break;
      case 'b':
        options.bandwidth = atoi(optarg) * 1000 * 1000;  // Mbit
        break;
      case 'i':
        options.interface = optarg;
        break;
      case 'p':
        char name[10];
        strcpy(name, optarg);
        for (int i = 0; i < strlen(name); i++) {
          name[i] = tolower(name[i]);
        }
        if (strcmp(name, "tcp") == 0) {
          options.protocol = Protocol::kTCP;
        } else if (strcmp(name, "raw") == 0) {
          options.protocol = Protocol::kRawSocket;
        } else {
          throw std::runtime_error(fmt::format("unknown protocol: {}", name));
        }
        break;
      default:
        throw std::runtime_error(
            fmt::format("unknown option: -{}", std::string(1, optopt)));
        break;
    }
  }

  if (optind < argc) {
    throw std::runtime_error("too many arguments");
  }

  return options;
}