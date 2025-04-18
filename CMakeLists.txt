cmake_minimum_required(VERSION 3.7.2)
project(PrakBS25 C)

set(CMAKE_C_STANDARD 11)

add_executable(PrakBS25 main.c
        keyValStore.c
        keyValStore.h
        main.h
        sub.c
        sub.h)
