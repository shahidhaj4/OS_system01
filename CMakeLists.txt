cmake_minimum_required(VERSION 3.28.3)
project(OS_system01 C)

set(CMAKE_C_STANDARD 11)

add_compile_definitions(_XOPEN_SOURCE=700)
add_compile_definitions(_POSIX_C_SOURCE=200809L)

add_executable(sim
        Main.c
        file_reader.c
        dijkstra.c
        graph.c
        drawing.c
        animation.c
        sync.c
        sync.h
)

target_link_libraries(sim
        raylib
        m
        X11
        pthread
        dl
        rt
)