# Umicom Studio IDE
# File: cmake/UmicomStudioSourceCheck.cmake
# PURPOSE: Compose the optional native source-check controller and tool.
# AUTHOR AND ORGANISATION: Sammy Hegab / Umicom Foundation
# LICENCE: MIT

# The caller selects a Framework document-check target. This small composition
# has no dependency on Studio bootstrap, GTK or application launch services.
function(umicom_studio_add_source_check adapter)
    get_filename_component(_studio "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    add_library(umicom_studio_source_check STATIC "${_studio}/src/app/source_check.c")
    target_include_directories(umicom_studio_source_check PUBLIC "${_studio}/include")
    target_link_libraries(umicom_studio_source_check PUBLIC "${adapter}")
    target_compile_features(umicom_studio_source_check PUBLIC c_std_23)
    add_executable(umicom-studio-source-check "${_studio}/src/tools/source_check_main.c")
    target_link_libraries(umicom-studio-source-check PRIVATE umicom_studio_source_check)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(umicom_studio_source_check PRIVATE -Wall -Wextra -Wpedantic -Werror)
        target_compile_options(umicom-studio-source-check PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-check)
    endif()
endfunction()
