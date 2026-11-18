#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/cmake/learning_regeneration/CMakeLists.txt
#
# PURPOSE:
#   Verify production learning manifest stability, content regeneration and staging without full application dependencies.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.25)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(UmicomStudioLearningRegeneration LANGUAGES C)
    enable_testing()
endif()
get_filename_component(_studio_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT DEFINED UMICOM_REGEN_FRAMEWORK_ROOT)
    get_filename_component(UMICOM_REGEN_FRAMEWORK_ROOT "${_studio_root}/../../framework" ABSOLUTE)
endif()
add_test(NAME studio.cmake.learning_regeneration
    COMMAND "${CMAKE_COMMAND}"
        "-DFRAMEWORK_ROOT=${UMICOM_REGEN_FRAMEWORK_ROOT}"
        "-DSTUDIO_ROOT=${_studio_root}"
        "-DFIXTURE=${CMAKE_CURRENT_LIST_DIR}/fixture"
        "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/isolated"
        "-DGENERATOR=${CMAKE_GENERATOR}"
        "-DMAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        "-DC_COMPILER=${CMAKE_C_COMPILER}"
        -P "${CMAKE_CURRENT_LIST_DIR}/check.cmake")
set_tests_properties(studio.cmake.learning_regeneration PROPERTIES
    TIMEOUT 120 LABELS "studio;cmake;learning;regeneration")
