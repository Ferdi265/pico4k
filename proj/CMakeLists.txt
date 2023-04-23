set(PROJ_NAME pico4k)

add_library(${PROJ_NAME}_common INTERFACE)
target_include_directories(${PROJ_NAME}_common INTERFACE include/)
target_compile_definitions(${PROJ_NAME}_common INTERFACE
    PICOPACK_WITH_PICO_MEM_OPS=1
    WANT_VBLANK_IRQ=1
)

usdk_generate_pio_header(${PROJ_NAME}_common src/vga_timing.pio)
usdk_generate_pio_header(${PROJ_NAME}_common src/vga_pixels.pio)

file(GLOB_RECURSE sources src/*.cpp src/*.s src/*.S)
add_executable(${PROJ_NAME}_main ${sources})
target_link_libraries(${PROJ_NAME}_main PRIVATE
    picopack_part_common ${PROJ_NAME}_common
    hardware_regs hardware_structs
    hardware_gpio hardware_dma hardware_pio hardware_divider hardware_pwm
    usdk pico_mem_ops
)

add_executable(${PROJ_NAME})
picopack_link(${PROJ_NAME} upkr2ram PARTS
    ${PROJ_NAME}_main
)
