cmake_minimum_required(VERSION 3.23)
project(Security C)

set(CMAKE_C_STANDARD 99)

add_executable(Security main.c constants.h seridog.h string_support.h)
