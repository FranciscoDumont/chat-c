cmake_minimum_required(VERSION 3.14)
project(Server)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(Server server.c)

target_link_libraries(Server
        "/usr/lib/libaltaLibreria.so"
        "/usr/lib/libcommons.so"
        pthread
        m)
