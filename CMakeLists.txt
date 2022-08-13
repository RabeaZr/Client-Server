cmake_minimum_required(VERSION 3.17)
project(OS5 C)

set(CMAKE_C_STANDARD 11)

add_executable(OS5 pcc_server.c pcc_client.c)