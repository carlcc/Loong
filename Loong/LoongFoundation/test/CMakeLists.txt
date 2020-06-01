add_executable(LoongFoundation_unittest Test.cpp)

target_link_libraries(LoongFoundation_unittest
PUBLIC
    LoongFoundation
)

set_target_properties(LoongFoundation_unittest PROPERTIES
    FOLDER Loong_unittests
)