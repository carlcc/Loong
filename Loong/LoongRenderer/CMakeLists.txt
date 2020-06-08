cmake_minimum_required (VERSION 3.2)

project(LoongRenderer)


file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongRenderer STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongRenderer
PUBLIC
    include
)

target_link_libraries(LoongRenderer
PUBLIC
    LoongFoundation
    LoongResource
    glad
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongRenderer PROPERTIES
    FOLDER Loong
)
