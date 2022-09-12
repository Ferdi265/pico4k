function(add_tool name executable)
    include(ExternalProject)

    set(TOOL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${name})
    set(TOOL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${name})

    set(TOOL_BUILD_TARGET ${name})

    ExternalProject_Add(${TOOL_BUILD_TARGET}
        PREFIX exomizer SOURCE_DIR ${TOOL_SOURCE_DIR}
        BINARY_DIR ${TOOL_BINARY_DIR}
        BUILD_ALWAYS 1
        INSTALL_COMMAND ""
    )

    string(TOUPPER "${executable}" executable_upper)
    set(ENV{${executable_upper}} ${TOOL_BINARY_DIR}/${executable})
endfunction()

add_tool(exomizer exoraw)
