cmake_minimum_required (VERSION 3.6)

project(LoongGui)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongGui STATIC ${SOURCE} ${INCLUDE})
set_loong_common_target_properties(LoongGui)

target_include_directories(LoongGui
PUBLIC
    include
    )
target_link_libraries(LoongGui
PUBLIC
    Diligent-Imgui
    Diligent-Common
    LoongFoundation
)


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongGui PROPERTIES
FOLDER
    Loong
    )

