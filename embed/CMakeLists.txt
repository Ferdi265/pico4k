set(EMBED_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

function(embed_file target file)
    cmake_parse_arguments(embed_file "" "ATTRIBUTES" "" ${ARGN} )

    if(embed_file_ATTRIBUTES)
        set(attributes " ${embed_file_ATTRIBUTES}")
    else()
        set(attributes " ")
    endif()

    cmake_path(GET file STEM embed-base)
    cmake_path(GET file EXTENSION embed-ext)
    set(embed-name "${embed-base}${embed-ext}")
    string(REGEX REPLACE "\\." "_" embed-name "${embed-name}")

    set(embed-header-template "${EMBED_DIR}/embed.hpp.in")
    set(embed-source-template "${EMBED_DIR}/embed.cpp.in")
    set(embed-header "${CMAKE_CURRENT_BINARY_DIR}/include/${embed-name}.hpp")
    set(embed-source "${CMAKE_CURRENT_BINARY_DIR}/src/${embed-name}.cpp")

    message(STATUS "embedding ${embed-base}${embed-ext}")

    string(REGEX REPLACE "([()])" "\\\\\\1" attributes "${attributes}")
    add_custom_command(
        OUTPUT "${embed-source}"
        MAIN_DEPENDENCY "${file}"
        COMMAND "${CMAKE_COMMAND}" -P "${EMBED_DIR}/embed.cmake" "${file}" "${embed-source-template}" "${embed-source}" "${attributes}"
    )
    add_custom_target(gen-${embed-name} DEPENDS "${embed-source}")

    set(FILENAME "${embed-name}")
    configure_file("${embed-header-template}" "${embed-header}" @ONLY)

    set_source_files_properties("${embed-header}" PROPERTIES GENERATED 1)
    set_source_files_properties("${embed-source}" PROPERTIES GENERATED 1)

    add_dependencies(${target} gen-${embed-name})
    target_sources(${target} PRIVATE "${embed-source}")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include/")
endfunction()
