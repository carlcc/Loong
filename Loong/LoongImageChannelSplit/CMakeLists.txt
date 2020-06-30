cmake_minimum_required(VERSION 3.2)

project(LoongImageSplit CXX)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

set(ASSETS

        )

add_executable(LoongImageSplit ${SOURCE} ${INCLUDE})
target_include_directories(LoongImageSplit PRIVATE src)

target_link_libraries(LoongImageSplit
PRIVATE
        LoongFoundation
        )

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongImageSplit PROPERTIES
        FOLDER Loong
)
        