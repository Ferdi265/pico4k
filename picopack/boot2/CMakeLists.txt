add_library(picopack_boot2_headers INTERFACE)
target_include_directories(picopack_boot2_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(picopack_boot2_headers INTERFACE hardware_regs)
target_compile_definitions(picopack_boot2_headers INTERFACE
    $<IF:$<BOOL:$<TARGET_PROPERTY:PICOPACK_TARGET_BOOT2>>,PICOPACK_BOOT2_$<UPPER_CASE:$<TARGET_PROPERTY:PICOPACK_TARGET_BOOT2>>,>
)

function(picopack_define_boot2 name sources)
    add_executable(${name} ${sources})
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_link_options(${name} PRIVATE "-nostdlib")
    else()
        target_link_options(${name} PRIVATE "--specs=nosys.specs")
        target_link_options(${name} PRIVATE "-nostartfiles")
    endif()

    target_link_libraries(${name} PRIVATE picopack_boot2_headers)
    target_link_options(${name} PRIVATE "LINKER:--script=${PICO_BOOT_STAGE2_DIR}/boot_stage2.ld")
    set_target_properties(${name} PROPERTIES LINK_DEPENDS ${PICO_BOOT_STAGE2_DIR}/boot_stage2.ld)
    set_target_properties(${name} PROPERTIES PICOPACK_TARGET_BOOT2 ${name})

    pico_add_dis_output(${name})
    pico_add_map_output(${name})

    set(original_bin ${CMAKE_CURRENT_BINARY_DIR}/${name}.bin)
    set(padded_src ${CMAKE_CURRENT_BINARY_DIR}/${name}_padded.S)

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    add_custom_target(${name}_bin DEPENDS ${original_bin})
    add_custom_command(OUTPUT ${original_bin} DEPENDS ${name} COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${name}> ${original_bin})

    add_custom_target(${name}_padded_src DEPENDS ${padded_src})
    add_custom_command(OUTPUT ${padded_src} DEPENDS ${original_bin}
        COMMAND ${Python3_EXECUTABLE} ${PICO_BOOT_STAGE2_DIR}/pad_checksum -s 0xffffffff ${original_bin} ${padded_src}
    )

    add_library(${name}_library INTERFACE)
    add_dependencies(${name}_library ${name}_padded_src)
    target_link_libraries(${name}_library INTERFACE ${padded_src})
endfunction()

function(picopack_set_boot2 target name)
    set_target_properties(${target} PROPERTIES PICOPACK_TARGET_BOOT2 ${name})
endfunction()

picopack_define_boot2(copy2ram src/copy2ram.S)
picopack_define_boot2(exo2ram src/exo2ram.S)
picopack_define_boot2(upkr2ram src/upkr2ram.S)
