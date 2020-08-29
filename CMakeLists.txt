cmake_minimum_required(VERSION 3.10)
project(socket_test)

set(CMAKE_CXX_STANDARD 11)

link_libraries(pthread)

add_executable(server server.cpp)

add_executable(client client.cpp config.h)

