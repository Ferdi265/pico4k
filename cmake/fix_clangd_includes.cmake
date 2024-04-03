set(fix-clangd-c-includes "")
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    execute_process(
        COMMAND "${CMAKE_C_COMPILER}" -x c -Wp,-v -E -
        INPUT_FILE /dev/null
        ERROR_VARIABLE fix-clangd-c-includes-output
        OUTPUT_QUIET
    )
    string(REPLACE "\n" ";" fix-clangd-c-includes-lines "${fix-clangd-c-includes-output}")
    list(FILTER fix-clangd-c-includes-lines INCLUDE REGEX "^ .*")
    list(TRANSFORM fix-clangd-c-includes-lines STRIP)
    set(fix-clangd-c-includes "${fix-clangd-c-includes-lines}")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    # nothing to do
else()
    message(WARNING "unsupported C compiler vendor ${CMAKE_C_COMPILER_ID}")
endif()

foreach(include ${fix-clangd-c-includes})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${include}")
    message(STATUS "fix_clangd_includes: found C default include ${include}")
endforeach()

set(fix-clangd-cpp-includes "")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -x c++ -Wp,-v -E -
        INPUT_FILE /dev/null
        ERROR_VARIABLE fix-clangd-cpp-includes-output
        OUTPUT_QUIET
    )
    string(REPLACE "\n" ";" fix-clangd-cpp-includes-lines "${fix-clangd-cpp-includes-output}")
    list(FILTER fix-clangd-cpp-includes-lines INCLUDE REGEX "^ .*")
    list(TRANSFORM fix-clangd-cpp-includes-lines STRIP)
    set(fix-clangd-cpp-includes "${fix-clangd-cpp-includes-lines}")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # nothing to do
else()
    message(WARNING "unsupported C++ compiler vendor ${CMAKE_CXX_COMPILER_ID}")
endif()

foreach(include ${fix-clangd-cpp-includes})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${include}")
    message(STATUS "fix_clangd_includes: found C++ default include ${include}")
endforeach()
