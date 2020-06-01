cmake_minimum_required (VERSION 3.6)

project(LoongFoundation)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongFoundation STATIC ${SOURCE} ${INCLUDE})
set_loong_common_target_properties(LoongFoundation)

target_include_directories(LoongFoundation
PUBLIC
    include
)


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongFoundation PROPERTIES
    FOLDER Loong
)


add_subdirectory(test)
