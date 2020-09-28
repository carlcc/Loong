cmake_minimum_required (VERSION 3.2)

project(LoongResource)


file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongResource STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongResource
PUBLIC
    include
)

target_link_libraries(LoongResource
PUBLIC
    LoongFoundation
    LoongAsset
    LoongFileSystem
    LoongRHI
    glad
    jsonmapper
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongResource PROPERTIES
    FOLDER Loong
)
