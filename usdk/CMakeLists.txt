# --- usdk header-only library ---

add_library(usdk-header-only INTERFACE)
target_include_directories(usdk-header-only INTERFACE include/)
target_link_libraries(usdk-header-only INTERFACE
    hardware_gpio hardware_pio hardware_interp
    hardware_clocks
)

# --- usdk static library ---

add_library(usdk INTERFACE)
target_link_libraries(usdk INTERFACE
    usdk-header-only
)
target_compile_definitions(usdk INTERFACE
    PICO_NO_BINARY_INFO
    PICO_DISABLE_SHARED_IRQ_HANDLERS
)

file(GLOB sources src/*.cpp src/*.S src/*.s)
target_sources(usdk INTERFACE ${sources})

# --- pioasm extensions ---

file(GLOB pioasm-sources pioasm/*.cpp)
set(PIOASM_EXTRA_SOURCE_FILES ${pioasm-sources} PARENT_SCOPE)
function(usdk_init_pioasm)
    _pico_init_pioasm()
endfunction()

function(usdk_generate_pio_header TARGET PIO)
    _pico_init_pioasm()
    cmake_parse_arguments(usdk_generate_pio_header "" "OUTPUT_FORMAT;SUBDIR" "" ${ARGN} )

    if (usdk_generate_pio_header_OUTPUT_FORMAT)
        set(OUTPUT_FORMAT "${usdk_generate_pio_header_OUTPUT_FORMAT}")
    else()
        set(OUTPUT_FORMAT "cpp-sdk")
    endif()

    set(HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/pioasm")
    if (usdk_generate_pio_header_SUBDIR)
        set(SUBDIR "/${usdk_generate_pio_header_SUBDIR}")
    else()
        set(SUBDIR "")
    endif()
    file(MAKE_DIRECTORY "${HEADER_DIR}${SUBDIR}")

    get_filename_component(PIO_ABS ${PIO} ABSOLUTE)
    get_filename_component(PIO_NAME ${PIO} NAME)
    set(HEADER "${HEADER_DIR}${SUBDIR}/${PIO_NAME}.h")
    get_filename_component(HEADER_GEN_TARGET ${PIO} NAME_WE)
    set(HEADER_GEN_TARGET "${TARGET}_${HEADER_GEN_TARGET}_pio_h")

    add_custom_target(${HEADER_GEN_TARGET} DEPENDS ${HEADER})
    add_custom_command(
        OUTPUT ${HEADER}
        DEPENDS ${PIO_ABS}
        COMMAND Pioasm -o ${OUTPUT_FORMAT} ${PIO_ABS} ${HEADER}
    )
    add_dependencies(${TARGET} ${HEADER_GEN_TARGET})

    get_target_property(target_type ${TARGET} TYPE)
    if ("INTERFACE_LIBRARY" STREQUAL "${target_type}")
        target_include_directories(${TARGET} INTERFACE ${HEADER_DIR})
    else()
        target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
    endif()
endfunction()
