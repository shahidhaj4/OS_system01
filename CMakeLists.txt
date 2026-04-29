cmake_minimum_required(VERSION 3.28.3)
project(OS_system01 C)

set(CMAKE_C_STANDARD 11)

add_library(mylib
        library.c
        file_reader.c
        dijkstra.c
        graph.c
)

add_executable(OS_system01
        Main.c
        graph.c)

target_link_libraries(OS_system01 mylib)