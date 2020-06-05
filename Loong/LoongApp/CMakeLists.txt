project(LoongApp)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongApp STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongApp
PUBLIC
        include
        )
target_link_libraries(LoongApp
PUBLIC
        LoongFoundation
        glfw
        glad
        imgui
        )


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongApp PROPERTIES
FOLDER Loong
        )

