cmake_minimum_required(VERSION 3.11)
project("tinkov_lang" VERSION 1.0)

SET(SRCS
    src/backend/backend.cpp
    src/middleend/middleend.cpp
    src/frontend/frontend.cpp
    src/common/common.cpp
    src/compiler/compiler.cpp
    src/stack/stack.cpp
    src/my_hash.cpp
    src/stslib_tnkff/stdlib_tnkff.s
    src/tree/tree.cpp
    src/tree/tree_dump/tree_dump.cpp
)

add_library(common STATIC src/common.cpp)
target_include_directories(common PRIVATE include/)

add_library(hash STATIC src/my_hash.cpp)
target_include_directories(hash PRIVATE include/)

add_library(stack STATIC src/stack.cpp)
target_link_libraries(stack hash_lib)
target_include_directories(stack PRIVATE include/)

add_library(tree STATIC src/tree.cpp)
target_link_libraries(tree common)
target_include_directories(tree PRIVATE include/)

add_library(tree_dump STATIC src/tree_dump.cpp)
target_link_libraries(tree_dump tree)
target_include_directories(tree_dump PRIVATE include/)

# add_library(frontend src/frontend.cpp)
add_executable(frontend src/frontend.cpp)
target_link_libraries(frontend tree tree_dump)
target_include_directories(frontend PRIVATE include/)

# add_library(middleend src/middleend.cpp)
add_executable(middleend src/middleend.cpp)
target_link_libraries(middleend common tree tree_dump)
target_include_directories(middleend PRIVATE include/)

# add_library(backend src/backend.cpp)
add_executable(backend src/backend.cpp)
target_link_libraries(backend common tree tree_dump)
target_include_directories(backend PRIVATE include/)

add_executable(compiler src/compiler.cpp)
set_target_properties(compiler PROPERTIES OUTPUT_NAME "tinkov")
target_link_libraries(compiler common)
target_include_directories(compiler PRIVATE include/)
