cmake_minimum_required(VERSION 3.6)

project(PlayGround CXX)

set(SOURCE
        src/main.cpp
        )

set(INCLUDE

        )

set(ASSETS

        )

add_loong_app(PlayGround "Loong"  "${SOURCE}" "${INCLUDE}" "${ASSETS}")
set_loong_common_target_properties(PlayGround)
target_link_libraries(PlayGround
PRIVATE
        LoongGui
        )
