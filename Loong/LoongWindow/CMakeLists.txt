project(LoongWindow)

file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)


add_library(LoongWindow STATIC ${SOURCE} ${INCLUDE})

target_include_directories(LoongWindow
PUBLIC
        include
        )
target_link_libraries(LoongWindow
PUBLIC
        LoongFoundation
        glfw
        glad
        imgui
        )


source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

set_target_properties(LoongWindow PROPERTIES
FOLDER Loong
        )

