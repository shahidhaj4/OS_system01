cmake_minimum_required(VERSION 3.28.3)
project(OS_system01 C)

set(CMAKE_C_STANDARD 11)
add_executable(OS_system01
        main.c
        graph.c
        file_reader.c
        dijkstra.c
)

add_executable(OS_system01 Main.c library.c
        file_reader.c
        file_reader.h
)