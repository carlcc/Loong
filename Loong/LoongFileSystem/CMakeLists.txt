cmake_minimum_required (VERSION 3.2)

project(LoongFileSystem)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongFileSystem STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongFileSystem
PUBLIC
    include
)

target_link_libraries(LoongFileSystem
PUBLIC
    LoongFoundation
PRIVATE
    physfs-static
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongFileSystem PROPERTIES
    FOLDER Loong
)


add_subdirectory(test)
