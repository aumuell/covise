if(NOT COVISE_LINKER)
    set(COVISE_LINKER "Default" CACHE STRING "Configure linker" FORCE)
endif()
# Set the possible values for cmake-gui
set_property(CACHE COVISE_LINKER PROPERTY STRINGS "Default" "bfd" "gold" "lld")
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    if (COVISE_LINKER AND NOT COVISE_LINKER STREQUAL "Default")
        add_link_options(-fuse-ld=${COVISE_LINKER})
    endif()
endif()