function(picopack_add_part_library target part-target part-num)
    set(PICOPACK_PART_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR})

    set(original_bin ${CMAKE_CURRENT_BINARY_DIR}/${part-target}.bin)
    set(packed_bin ${CMAKE_CURRENT_BINARY_DIR}/${part-target}_packed.bin)
    set(packed_src ${CMAKE_CURRENT_BINARY_DIR}/${part-target}_packed.S)

    add_custom_target(${part-target}_bin DEPENDS ${original_bin})
    add_custom_command(OUTPUT ${original_bin} DEPENDS ${part-target} COMMAND
        "${CMAKE_OBJCOPY}" -Obinary $<TARGET_FILE:${part-target}> ${original_bin}
    )

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    add_custom_target(${part-target}_packed_bin DEPENDS ${packed_bin})
    add_custom_command(OUTPUT ${packed_bin} DEPENDS ${original_bin}
        COMMAND ${CMAKE_COMMAND} -E env "EXORAW=$ENV{EXORAW}" env "UPKR=$ENV{UPKR}" "${Python3_EXECUTABLE}"
        "${PICOPACK_PART_DIR}/part_$<TARGET_PROPERTY:${target},PICOPACK_TARGET_BOOT2>.py" ${original_bin} ${packed_bin}
    )
    add_dependencies(${part-target}_packed_bin exomizer)

    add_custom_target(${part-target}_packed_src DEPENDS ${packed_src})
    add_custom_command(OUTPUT ${packed_src} DEPENDS ${packed_bin}
        COMMAND ${Python3_EXECUTABLE} ${PICOPACK_PART_DIR}/embed_part.py ${packed_bin} ${part-num} ${packed_src}
    )

    add_library(${part-target}_library INTERFACE)
    add_dependencies(${part-target}_library ${part-target}_packed_src)
    target_link_libraries(${part-target}_library INTERFACE ${packed_src})
endfunction()

file(GLOB_RECURSE part-common-sources src/*.cpp src/*.s src/*.S)
add_library(picopack_part_common INTERFACE)
target_sources(picopack_part_common INTERFACE ${part-common-sources})
target_include_directories(picopack_part_common INTERFACE include/)

target_link_libraries(picopack_part_common INTERFACE
    hardware_regs hardware_structs
    picopack_boot2_headers
)
