cmake_minimum_required (VERSION 3.6)

project(LoongAsset)


file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongAsset STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongAsset
PUBLIC
    include
)

target_link_libraries(LoongAsset
PRIVATE
    LoongFileSystem
    LoongFoundation
    FakeAssimp
)

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongAsset PROPERTIES
    FOLDER Loong
)
