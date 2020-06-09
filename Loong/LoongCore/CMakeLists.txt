cmake_minimum_required (VERSION 3.2)

project(LoongCore)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongCore STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongCore
PUBLIC
    include
)

target_link_libraries(LoongCore
PUBLIC
    LoongFoundation
    LoongAsset
    LoongRenderer
    LoongResource
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongCore PROPERTIES
    FOLDER Loong
)

