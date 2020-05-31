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
if (WIN32)
    copy_required_dlls(PlayGround)
    add_custom_command(TARGET PlayGround POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "\"$<TARGET_FILE:gainput>\""
        "\"$<TARGET_FILE_DIR:PlayGround>\"")
    append_loong_app_base_win32_source(PlayGround)
endif()

set_target_properties(LoonAppBase PROPERTIES
        FOLDER Loong
        )