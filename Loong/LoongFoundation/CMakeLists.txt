cmake_minimum_required (VERSION 3.6)

project(LoongFoundation)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongFoundation STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongFoundation
PUBLIC
    include
    ${GLM_INCLUDE_DIR}
)


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongFoundation PROPERTIES
    FOLDER Loong
)


add_subdirectory(test)
