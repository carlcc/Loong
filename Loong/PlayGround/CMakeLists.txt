cmake_minimum_required(VERSION 3.6)

project(PlayGround CXX)

# We need to add WIN32 flag to make the compiler link to WinMain. See
# https://cmake.org/cmake/help/v3.8/command/add_executable.html
add_executable(PlayGround ${LOONG_WIN32_FLAG} src/main.cpp)
target_link_libraries(PlayGround PRIVATE
        Diligent-BuildSettings
        Diligent-NativeAppBase
        LoonAppBase
        )

set_common_target_properties(PlayGround)
copy_required_dlls(PlayGround)
append_loong_app_base_win32_source(PlayGround)