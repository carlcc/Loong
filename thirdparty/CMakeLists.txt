
set(CMAKE_CXX_STANDARD 17)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

add_subdirectory(physfs EXCLUDE_FROM_ALL)

add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(glad EXCLUDE_FROM_ALL)
