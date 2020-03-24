cmake_minimum_required(VERSION 3.14)
project(Cliente)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(Cliente cliente.c)

target_link_libraries(Cliente
        "/usr/lib/libaltaLibreria.so"
        "/usr/lib/libcommons.so"
        pthread
        m)
