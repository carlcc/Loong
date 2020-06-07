cmake_minimum_required (VERSION 3.6)

project(LoongGui)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongGui STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongGui
PUBLIC
    include
    )
target_link_libraries(LoongGui
PUBLIC
    imgui
    LoongFoundation
    LoongResource
)


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongGui PROPERTIES
FOLDER
    Loong
    )

