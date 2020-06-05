cmake_minimum_required(VERSION 3.2)

project(Loong)

add_subdirectory(thirdparty)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm/include)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Loong)
