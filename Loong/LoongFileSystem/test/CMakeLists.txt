add_executable(LoongFileSystem_unittest Test.cpp)

target_link_libraries(LoongFileSystem_unittest
PUBLIC
    LoongFileSystem
)

set_target_properties(LoongFileSystem_unittest PROPERTIES
    FOLDER Loong_unittests
)