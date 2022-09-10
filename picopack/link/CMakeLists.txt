function(picopack_set_linker_script target ldscript)
    set_target_properties(${target} PROPERTIES PICOPACK_TARGET_LINKER_SCRIPT ${ldscript})
    pico_add_link_depend(${target} ${ldscript})
endfunction()

# == picopack_link_part ========================================================
add_library(picopack_link_part INTERFACE)
pico_add_map_output(picopack_link_part)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_link_options(picopack_link_part INTERFACE "LINKER:-nostdlib")
endif()

target_link_options(picopack_link_part INTERFACE --specs=nosys.specs -nostartfiles LINKER:--gc-sections)
target_link_options(picopack_link_part INTERFACE
    "LINKER:--script=$<IF:$<BOOL:$<TARGET_PROPERTY:PICOPACK_TARGET_LINKER_SCRIPT>>,$<TARGET_PROPERTY:PICOPACK_TARGET_LINKER_SCRIPT>,${CMAKE_CURRENT_SOURCE_DIR}/part.ld>"
)
target_compile_options(picopack_link_part INTERFACE -ffunction-sections -fdata-sections -fno-common)
target_cxx_compile_options(picopack_link_part INTERFACE -fno-threadsafe-statics -fno-exceptions -fno-unwind-tables -fno-rtti -fno-use-cxa-atexit)
pico_add_link_depend(picopack_link_part ${CMAKE_CURRENT_SOURCE_DIR}/part.ld)

# == picopack_link_final =======================================================
add_library(picopack_link_final INTERFACE)
pico_add_map_output(picopack_link_final)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_link_options(picopack_link_final INTERFACE "LINKER:-nostdlib")
endif()

target_link_options(picopack_link_final INTERFACE -nostartfiles)
target_link_options(picopack_link_final INTERFACE
    "LINKER:--script=${CMAKE_CURRENT_SOURCE_DIR}/final.ld"
)
pico_add_link_depend(picopack_link_final ${CMAKE_CURRENT_SOURCE_DIR}/final.ld)

target_link_libraries(picopack_link_final INTERFACE
    "$<TARGET_PROPERTY:PICOPACK_TARGET_BOOT2>_library"
)
target_sources(picopack_link_final INTERFACE src/dummy.S)
