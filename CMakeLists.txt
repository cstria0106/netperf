cmake_minimum_required(VERSION 3.10)

project(netperf)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES src/main.cpp src/server.cpp src/client.cpp src/receive.cpp src/send.cpp src/tcp.cpp src/raw_socket.cpp src/udp.cpp src/report.cpp)

add_executable(netperf ${SOURCES})

add_subdirectory(fmt)
target_link_libraries(netperf PRIVATE fmt::fmt)
