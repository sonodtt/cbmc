file(GLOB_RECURSE sources "*.cpp" "*.h")

list(REMOVE_ITEM sources
    # Remove usage of java_bytecode
    ${CMAKE_CURRENT_SOURCE_DIR}/load_java_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/load_java_class.h
)

add_library(testing-utils ${sources})
target_link_libraries(testing-utils
    util
)
target_include_directories(testing-utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
