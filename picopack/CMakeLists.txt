add_subdirectory(boot2)
add_subdirectory(part)
add_subdirectory(link)

include(CMakeParseArguments)

function(picopack_link target boot2)
    cmake_parse_arguments(picopack_link "POSTFIX_DATA" "" "PARTS" ${ARGN})

    target_link_libraries(${target} PRIVATE picopack_link_final)
    picopack_set_boot2(${target} ${boot2})
    pico_add_extra_outputs(${target})

    set(part-num 0)
    list(LENGTH picopack_link_PARTS num-parts)
    foreach(part ${picopack_link_PARTS})
        if(NOT picopack_link_POSTFIX_DATA AND ${num-parts} LESS_EQUAL 1)
            target_compile_definitions(${part} PRIVATE PICOPACK_NO_NEXT_PART=1)
        endif()

        target_link_libraries(${part} PRIVATE picopack_link_part picopack_boot2_headers)
        picopack_set_boot2(${part} ${boot2})
        picopack_add_part_library(${target} ${part} ${part-num})

        target_link_libraries(${target} PRIVATE ${part}_library)
        math(EXPR part-num "${part-num}+1")
    endforeach()
endfunction()
