cmake_minimum_required(VERSION 3.2)

project(LoongAssetConverter CXX)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

set(ASSETS

        )

add_executable(LoongAssetConverter ${SOURCE} ${INCLUDE})
target_include_directories(LoongAssetConverter PRIVATE src)

target_link_libraries(LoongAssetConverter
PRIVATE
        LoongAsset
        LoongFileSystem
        LoongResource
        FakeAssimp
        )

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongAssetConverter PROPERTIES
        FOLDER Loong
)
        