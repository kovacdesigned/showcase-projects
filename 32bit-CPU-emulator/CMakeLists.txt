cmake_minimum_required(VERSION 3.10)
project(simple_cpu_emulator C)

set(CMAKE_C_STANDARD 99)

add_executable(cpu
    main.c
    cpu.c
    cpu.h
)
