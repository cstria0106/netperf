#pragma once

#include "fmt/core.h"
#include "test.h"
#include "util.h"
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
  char const* interface;
  int window_size;
  bool no_delay;
  bool zero_copy;
};

inline Options ParseOptions(int argc, char** argv) {
  Options options;
  options.server = false;
  options.client = false;
  options.protocol = Protocol::kTCP;
  options.address = nullptr;
  options.server = false;
  options.receiver = false;
  options.time = 10;
  options.block_len = 1000;
  options.bandwidth = 0;
  options.interface = "lo";
  options.window_size = 0;
  options.no_delay = false;

  opterr = 0;
  signed char option;
  while ((option = getopt(argc, argv, "sc:Rt:l:b:ur:w:Nz")) != -1) {
    switch (option) {
      case 's':
        options.server = true;
        break;
      case 'c':
        options.client = true;
        options.address = optarg;
        break;
      case 'R':
        options.receiver = true;
        break;
      case 't':
        options.time = atoi(optarg);
        break;
      case 'l':
        options.block_len = ParseBytes(optarg);
        break;
      case 'b':
        options.bandwidth = ParseBytes(optarg);
        break;
      case 'w':
        options.window_size = ParseBytes(optarg);
        break;
      case 'u':
        options.protocol = Protocol::kUDP;
        break;
      case 'r':
        options.protocol = Protocol::kRawSocket;
        options.interface = optarg;
        break;
      case 'z':
        options.zero_copy = true;
        break;
      case 'N':
        options.no_delay = true;
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