cmake_minimum_required(VERSION 3.24)
project(OfflineMessenger)

set(CMAKE_CXX_STANDARD 23)

add_executable(OfflineMessenger server.cpp client.cpp write_read_mess.h write_read_mess.h)
