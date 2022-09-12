cmake_minimum_required(VERSION 3.14)
project(exomizer C)

file(GLOB exomizer-sources *.c)
add_executable(exoraw ${exomizer-sources})
target_compile_options(exoraw PRIVATE -std=c89 -Wall -Wstrict-prototypes -D_XOPEN_SOURCE=600 -pedantic)

set($ENV{EXORAW} $<TARGET_FILE:exoraw>)
