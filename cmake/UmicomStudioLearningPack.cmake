#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioLearningPack.cmake
#
# PURPOSE:
#   Package the reviewed Studio learning guide as an explicit local catalogue.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#---------------------------------------------------------------------------

include_guard(GLOBAL)
include(GNUInstallDirs)
get_filename_component(_studio_learning_source "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(_studio_learning_pack_dir "${CMAKE_CURRENT_BINARY_DIR}/learning-pack")
file(MAKE_DIRECTORY "${_studio_learning_pack_dir}")
# Hash exactly the copied installation bytes. Source edits trigger CMake's
# normal configure dependency; no private workspace path enters the manifest.
configure_file("${_studio_learning_source}/docs/learning-in-studio.html"
    "${_studio_learning_pack_dir}/learning-in-studio.html" COPYONLY)
file(SHA256 "${_studio_learning_pack_dir}/learning-in-studio.html" _studio_learning_guide_sha256)
configure_file("${_studio_learning_source}/docs/learning-pack.json.in"
    "${_studio_learning_pack_dir}/learning-pack.json" @ONLY)
install(FILES
    "${_studio_learning_pack_dir}/learning-pack.json"
    "${_studio_learning_pack_dir}/learning-in-studio.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/learning-pack")

if(BUILD_TESTING AND TARGET Umicom::learning_catalogue_gtk4
        AND TARGET umicom-studio-learning-locations-test)
    foreach(_case catalogue-guide catalogue-guide-retained)
        add_test(NAME "studio.learning.locations.${_case}"
            COMMAND umicom-studio-learning-locations-test "${_case}" "${_studio_learning_pack_dir}")
        set_tests_properties("studio.learning.locations.${_case}" PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;learning;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.learning.locations.${_case}"
                TARGETS umicom-studio-learning-locations-test)
        endif()
    endforeach()
endif()
