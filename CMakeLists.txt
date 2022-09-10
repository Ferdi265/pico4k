cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/find_sdk.cmake)
include(cmake/pico_sdk_import.cmake)
include(cmake/cxx_compile_options.cmake)
project(picopack-sdk C CXX ASM)

# initialize SDK
pico_sdk_init()
include(cmake/fix_clangd_includes.cmake)

set(CMAKE_CXX_STANDARD 20)
add_cxx_compile_options(-Wno-volatile -Wno-deprecated-volatile) # silence C++20 warnings in SDK

add_subdirectory(picopack)
add_subdirectory(usdk)
add_subdirectory(embed)
usdk_init_pioasm()
add_subdirectory(proj)
