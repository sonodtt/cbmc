file(GLOB_RECURSE sources "*.cpp" "*.h")

file(GLOB_RECURSE testing_utils "testing-utils/*.cpp" "testing-utils/*.h")

file(GLOB_RECURSE java_bytecode "java_bytecode/*.cpp" "java_bytecode/*.h")
file(GLOB_RECURSE goto_program_generics
    "goto-programs/goto_program_generics/*.cpp"
    "goto-programs/goto_program_generics/*.h"
)

list(REMOVE_ITEM sources
    # Used in executables
    ${CMAKE_CURRENT_SOURCE_DIR}/miniBDD.cpp

    # Don't build
    ${CMAKE_CURRENT_SOURCE_DIR}/sharing_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/elf_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/smt2_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/osx_fat_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unicode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_scanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/float_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ieee_float.cpp

    # Will be built into a separate library and linked
    ${testing_utils}

    # java_bytecode usage is being remove
    ${java_bytecode}
    ${goto_program_generics}
    ${CMAKE_CURRENT_SOURCE_DIR}/analyses/dependence_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/goto-programs/class_hierarchy_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/goto-programs/class_hierarchy_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/goto-programs/remove_virtual_functions_without_fallback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pointer-analysis/custom_value_set_analysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solvers/refinement/string_constraint_instantiation/instantiate_not_contains.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solvers/refinement/string_refinement/dependency_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solvers/refinement/string_refinement/string_symbol_resolution.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/has_subtype.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/parameter_indices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/simplify_expr.cpp


    # Intended to fail to compile
    ${CMAKE_CURRENT_SOURCE_DIR}/util/expr_cast/expr_undefined_casts.cpp
)

add_subdirectory(testing-utils)

add_executable(unit ${sources})
target_include_directories(unit
    PUBLIC
    ${CBMC_BINARY_DIR}
    ${CBMC_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
        unit
        testing-utils
        ansi-c
        solvers
        goto-programs
        goto-instrument-lib
)

add_test(
    NAME unit
    COMMAND $<TARGET_FILE:unit>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(unit PROPERTIES LABELS "CORE;CBMC")

add_executable(miniBDD miniBDD.cpp)
target_include_directories(miniBDD
    PUBLIC
    ${CBMC_BINARY_DIR}
    ${CBMC_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(miniBDD solvers ansi-c)
add_test(NAME miniBDD COMMAND $<TARGET_FILE:miniBDD>)
set_tests_properties(miniBDD PROPERTIES LABELS "CORE;CBMC")
