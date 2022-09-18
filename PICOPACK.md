# PICOPACK

Executable packer for Raspberry Pi Pico.

## Features

- Depacker fits into `boot2`
- Supports jumping back into the depacker and loading further parts
- CMake-based interface to build demos with

## Dependencies

- for copy2ram: nothing, but this also doesn't pack anything
- for exo2ram: [exomizer](https://bitbucket.org/magli143/exomizer) (designed for exomizer 3.1.1, expected to be in PATH when compiling)
- for upkr2ram: [upkr](https://github.com/exoticorn/upkr) (designed for upkr 0.1.0, commit 629c5fc, expected to be in PATH when compiling)

## Example

- initialize `pico-sdk`
- optional: add vendored exomizer implementation with `add_subdirectory(tools)`
- load packer with `add_subdirectory(picopack)`
- create your parts with `add_executable` and link with `picopack_part_common`
- create your final binary with `add_executable(finalname)`
- call the packer with `picopack_link(finalname exo2ram PARTS ...)`

```cmake
cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)
project(picopack-test C CXX ASM)

pico_sdk_init()

add_subdirectory(tools)
add_subdirectory(picopack)

file(GLOB part1-src part1-src/*.cpp)
add_executable(part1 ${part1-src})
target_link_libraries(part1 PRIVATE
    picopack_part_common ...
)

file(GLOB part2-src part2-src/*.cpp)
add_executable(part2 ${part2-src})
target_link_libraries(part2 PRIVATE
    picopack_part_common ...
)

file(GLOB part3-src part3-src/*.cpp)
add_executable(part3 ${part3-src})
target_link_libraries(part3 PRIVATE
    picopack_part_common ...
)

add_executable(intro)
picopack_link(intro exo2ram PARTS
    part1 part2 part3
)
```

## API Documentation

API Documentation for picopack will follow soon-ish.
