# Umicom Studio IDE
# File: tests/source_check/CMakeLists.txt
# PURPOSE: Qualify the optional Studio controller and tool without starting the application.
# AUTHOR AND ORGANISATION: Sammy Hegab / Umicom Foundation
# LICENCE: MIT

cmake_minimum_required(VERSION 3.25)
project(UmicomStudioSourceChecks LANGUAGES C)
include(CTest)
get_filename_component(_studio "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
get_filename_component(_framework "${_studio}/../../framework" ABSOLUTE)
# Reuse the real Framework document/store/compiler composition and its checks.
add_subdirectory("${_framework}/tests/toolchain/document_check" framework-document-checks)
target_sources(document-check-platform PRIVATE
    "${_framework}/src/platform/input_file.c"
    "${_framework}/src/platform/output_file.c"
    "${_framework}/src/base/status.c")
include("${_studio}/cmake/UmicomStudioSourceCheck.cmake")
umicom_studio_add_source_check(document-check-subject)
if(WIN32)
    enable_language(RC)
endif()
include("${_framework}/cmake/UmicomApplicationBranding.cmake")
umicom_apply_application_branding(TARGET umicom-studio-source-check
    PRODUCT_NAME "Umicom Studio Source Check" INTERNAL_NAME "umicom-studio-source-check"
    RESOURCE_ROOT "${_framework}/resources" NO_RUNTIME_DEPLOYMENT)
add_executable(umicom-studio-source-check-test "../test_source_check.c")
target_link_libraries(umicom-studio-source-check-test PRIVATE umicom_studio_source_check)
add_test(NAME studio.source-check.controller COMMAND umicom-studio-source-check-test)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixtures")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/warning.uc" "int main(void){int unused=7;return 42;}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/accepted.uc" "int main(void){return 42;}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/syntax.uc" "int main(void){return missing;}")
string(REPEAT " " 65537 _large)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtures/capacity.uc" "${_large}")
foreach(_case warning accepted syntax capacity missing relative help usage)
    add_test(NAME studio.source-check.command.${_case} COMMAND "${CMAKE_COMMAND}"
        "-DTOOL=$<TARGET_FILE:umicom-studio-source-check>"
        "-DFIXTURES=${CMAKE_CURRENT_BINARY_DIR}/fixtures" "-DCASE=${_case}"
        -P "${_studio}/tests/check_source_check_cli.cmake")
endforeach()

add_executable(umicom-studio-source-check-output-test "../test_source_check_output.c")
target_link_libraries(umicom-studio-source-check-output-test PRIVATE umicom_studio_source_check)
foreach(_case warning accepted syntax)
    add_test(NAME studio.source-check.output.${_case} COMMAND umicom-studio-source-check-output-test print "${CMAKE_CURRENT_BINARY_DIR}/fixtures/${_case}.uc")
endforeach()
add_test(NAME studio.source-check.output.flush COMMAND umicom-studio-source-check-output-test flush "${CMAKE_CURRENT_BINARY_DIR}/fixtures/accepted.uc")
