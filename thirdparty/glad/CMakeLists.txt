cmake_minimum_required(VERSION 3.2)
project(glad)

add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)
set_property(TARGET glad PROPERTY POSITION_INDEPENDENT_CODE ON)
