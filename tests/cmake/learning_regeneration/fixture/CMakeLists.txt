#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/cmake/learning_regeneration/fixture/CMakeLists.txt
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
project(UmicomStudioLearningRegenerationFixture LANGUAGES C)
set(BUILD_TESTING OFF)
set(UMICOM_STUDIO_BITS2BANKING_MODULE "" CACHE PATH "" FORCE)
# Only dependency targets and lesson payloads are synthetic. The manifest
# producer, checksum validation and module staging are the production files.
set(_proxy "${CMAKE_CURRENT_BINARY_DIR}/proxy")
file(MAKE_DIRECTORY "${_proxy}/cmake" "${_proxy}/docs/learning/studio-wiki")
file(CONFIGURE OUTPUT "${_proxy}/CMakeLists.txt"
    CONTENT "add_library(umicom_developer INTERFACE)\n" @ONLY)
file(CONFIGURE OUTPUT "${_proxy}/cmake/UmicomLearningModuleBundle.cmake"
    CONTENT "include([====[${FRAMEWORK_ROOT}/cmake/UmicomLearningModuleBundle.cmake]====])\n" @ONLY)
add_subdirectory("${_proxy}" proxy-build)
add_library(learning_catalogue INTERFACE)
add_library(Umicom::learning_catalogue_gtk4 ALIAS learning_catalogue)
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/main.c"
    CONTENT "int main(void) { return 0; }\n" @ONLY)
add_executable(umicom-studio-ide EXCLUDE_FROM_ALL "${CMAKE_CURRENT_BINARY_DIR}/main.c")
set_target_properties(umicom-studio-ide PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(_studio_learning_pack_dir "${CMAKE_CURRENT_BINARY_DIR}/studio-guide")
file(MAKE_DIRECTORY "${_studio_learning_pack_dir}")
file(CONFIGURE OUTPUT "${_studio_learning_pack_dir}/guide.html"
    CONTENT "<!doctype html><title>Fixture guide</title><p>Read a guide.</p>\n" @ONLY)
file(SHA256 "${_studio_learning_pack_dir}/guide.html" _guide_hash)
file(CONFIGURE OUTPUT "${_studio_learning_pack_dir}/learning-pack.json"
    CONTENT "{\"schema\":\"umicom.learning.pack\",\"version\":1,\"id\":\"studio.learning-guide\",\"product\":\"studio\",\"resources\":[{\"path\":\"guide.html\",\"sha256\":\"${_guide_hash}\"}]}\n" @ONLY)
foreach(_page index first-program gtk-window framework-tour build-workflow extending-studio module-authoring source-reference glossary)
    if(NOT EXISTS "${_proxy}/docs/learning/studio-wiki/${_page}.html")
        file(WRITE "${_proxy}/docs/learning/studio-wiki/${_page}.html"
            "<!doctype html><title>${_page}</title><p>Fixture lesson.</p>\n")
    endif()
endforeach()
include("${STUDIO_ROOT}/cmake/UmicomStudioLearningModules.cmake")

# The shared writer must preserve literal text, including characters that a
# template expander or list parser could otherwise reinterpret.
set(_literal [=[@NOT_A_TEMPLATE@;${NOT_A_VARIABLE};café]=])
umicom_write_learning_generated_file("${CMAKE_CURRENT_BINARY_DIR}/literal generated.txt" "${_literal}")
file(READ "${CMAKE_CURRENT_BINARY_DIR}/literal generated.txt" _round_trip)
if(NOT _round_trip STREQUAL _literal)
    message(FATAL_ERROR "Shared generated-file writer changed literal content")
endif()

add_custom_target(check-noop COMMAND "${CMAKE_COMMAND}" -E echo "Fixture build reached its target")
