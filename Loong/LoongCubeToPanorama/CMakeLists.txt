cmake_minimum_required(VERSION 3.2)

project(LoongCubeToPanorama CXX)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

set(ASSETS

        )

add_executable(LoongCubeToPanorama ${SOURCE} ${INCLUDE})
target_include_directories(LoongCubeToPanorama PRIVATE src)

target_link_libraries(LoongCubeToPanorama
PRIVATE
        LoongFoundation
        LoongApp
        LoongResource
        LoongAsset
        )

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongCubeToPanorama PROPERTIES
        FOLDER Loong
)
        