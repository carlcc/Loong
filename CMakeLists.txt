cmake_minimum_required(VERSION 3.2)

project(Loong)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)

add_subdirectory(thirdparty)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm/include)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
else()
    add_compile_options("-Wall")
endif()

add_subdirectory(Loong)

file(COPY  ${CMAKE_CURRENT_LIST_DIR}/Resources DESTINATION  ${CMAKE_CURRENT_BINARY_DIR}/out)