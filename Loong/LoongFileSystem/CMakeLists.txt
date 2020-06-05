cmake_minimum_required (VERSION 3.6)

project(LoongFileSystem)

set(SOURCE
    src/LoongFileSystem/LoongFileSystem.cpp
)

set(INCLUDE
    include/LoongFileSystem/LoongFileSystem.h
)

add_library(LoongFileSystem STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongFileSystem
PUBLIC
    include
)

target_link_libraries(LoongFileSystem
PRIVATE
    physfs-static
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongFileSystem PROPERTIES
    FOLDER Loong
)


add_subdirectory(test)
