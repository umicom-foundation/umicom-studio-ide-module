#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioTranslation.cmake
#
# PURPOSE:
#   Compose and qualify Studio's optional native source translation command.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
include_guard(GLOBAL)

function(umicom_studio_add_translation platform_target)
    get_filename_component(_studio "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    set(_framework "${_studio}/../../framework")
    include("${_framework}/cmake/UmicomCPreview.cmake")
    umicom_add_cvm_file_translation(umicom_studio_cvm_translation "${platform_target}")
    add_executable(umicom-studio-translate "${_studio}/src/tools/translate_main.c")
    target_link_libraries(umicom-studio-translate PRIVATE umicom_studio_cvm_translation)
    set_target_properties(umicom-studio-translate PROPERTIES C_EXTENSIONS OFF)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(umicom-studio-translate PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
    endif()
    umicom_apply_application_branding(TARGET umicom-studio-translate
        PRODUCT_NAME "Umicom Studio Translate" INTERNAL_NAME "umicom-studio-translate"
        RESOURCE_ROOT "${_framework}/resources" NO_RUNTIME_DEPLOYMENT)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-translate)
    endif()
    if(BUILD_TESTING)
        foreach(_case help usage c-to-java java-to-c invalid existing same relative-input
                      relative-output missing oversize no-parent)
            set(_name "studio.source-translation.${_case}")
            add_test(NAME "${_name}" COMMAND "${CMAKE_COMMAND}"
                "-DTOOL=$<TARGET_FILE:umicom-studio-translate>" "-DCASE=${_case}"
                "-DROOT=${CMAKE_CURRENT_BINARY_DIR}/translation fixtures"
                -P "${_studio}/tests/check_translation_cli.cmake")
            set_tests_properties("${_name}" PROPERTIES LABELS "studio;translation;toolchain" TIMEOUT 30)
            if(COMMAND umicom_test_impact_owners)
                umicom_test_impact_owners("${_name}" TARGETS umicom-studio-translate)
            endif()
        endforeach()
    endif()
endfunction()
