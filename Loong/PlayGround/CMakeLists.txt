cmake_minimum_required(VERSION 3.6)

project(PlayGround CXX)

set(SOURCE
        src/main.cpp
        )

set(INCLUDE

        )

set(ASSETS

        )

add_executable(PlayGround ${SOURCE} ${INCLUDE})

target_link_libraries(PlayGround
PRIVATE
        LoongGui
        LoongApp
        LoongAsset
        LoongFileSystem
        LoongResource
        LoongRenderer
        LoongCore
        )
