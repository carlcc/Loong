cmake_minimum_required(VERSION 3.2)

project(LoongEditor CXX)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

set(ASSETS

        )

add_executable(LoongEditor ${SOURCE} ${INCLUDE})
target_include_directories(LoongEditor PRIVATE src)

target_link_libraries(LoongEditor
PRIVATE
        LoongWindow
        LoongAsset
        LoongFileSystem
        LoongResource
        LoongRenderer
        LoongCore
        nfd
        )

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongEditor PROPERTIES
        FOLDER Loong
)
        