add_executable(LoonFileSystem_unittest Test.cpp)

target_link_libraries(LoonFileSystem_unittest
PUBLIC
    LoongFileSystem
)

set_target_properties(LoonFileSystem_unittest PROPERTIES
    FOLDER Loong_unittests
)