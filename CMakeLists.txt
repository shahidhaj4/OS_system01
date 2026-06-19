cmake_minimum_required(VERSION 3.28.3)
project(OS_system01 C)

set(CMAKE_C_STANDARD 11)

add_compile_definitions(_XOPEN_SOURCE=700)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
add_compile_definitions(_DEFAULT_SOURCE)

add_executable(sim
        sim_main.c
        file_reader.c
        dijkstra.c
        graph.c
        drawing.c
        Dijkstra_draw.c
        sync.c
        sync.h
        scheduler.c
        scheduler.h
        queue.c
        queue.h
)

target_compile_definitions(sim PRIVATE MILESTONE=6)

target_link_libraries(sim
        raylib
        m
        X11
        pthread
        dl
        rt
)
