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
# Deliberate public guide allowlist: private plans and unrelated documentation
# are never swept into this pack by a directory glob. Existing guides remain
# their own source files; the generated pack owns exact copied bytes and hashes.
foreach(_guide EDITING_FILES READING_BUILD_ERRORS RUNNING_TESTS recent-projects studio-user-guide studio-menu-acceptance bits2banking-courses)
    configure_file("${_studio_learning_source}/docs/${_guide}.html"
        "${_studio_learning_pack_dir}/${_guide}.html" COPYONLY)
    file(SHA256 "${_studio_learning_pack_dir}/${_guide}.html" "_studio_guide_${_guide}_sha256")
endforeach()
configure_file("${_studio_learning_source}/docs/learning-pack.json.in"
    "${_studio_learning_pack_dir}/learning-pack.json" @ONLY)
install(FILES
    "${_studio_learning_pack_dir}/learning-pack.json"
    "${_studio_learning_pack_dir}/learning-in-studio.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/learning-pack")

if(BUILD_TESTING AND TARGET Umicom::learning_catalogue_gtk4
        AND TARGET umicom-studio-learning-locations-test)
    # A small fixture exercises the independent Bits2Banking identity without
    # requiring another repository or a private learning path during CTest.
    set(_studio_bits_learning_fixture "${CMAKE_CURRENT_BINARY_DIR}/bits2banking-learning-fixture")
    file(MAKE_DIRECTORY "${_studio_bits_learning_fixture}")
    configure_file("${_studio_learning_source}/docs/bits2banking-courses.html"
        "${_studio_bits_learning_fixture}/index.html" COPYONLY)
    file(SHA256 "${_studio_bits_learning_fixture}/index.html" _studio_bits_learning_sha256)
    configure_file("${_studio_learning_source}/tests/fixtures/bits2banking-learning-pack.json.in"
        "${_studio_bits_learning_fixture}/learning-pack.json" @ONLY)
    foreach(_case catalogue-guide catalogue-guide-retained
            catalogue-topic-editing catalogue-topic-errors catalogue-topic-tests
            catalogue-topic-switch catalogue-topic-busy catalogue-topic-retained catalogue-topic-recent
            welcome-library welcome-guide welcome-editing welcome-errors welcome-tests welcome-recent
            welcome-user-guide menu-user-guide welcome-bits2banking guided-bits2banking
            welcome-repeat welcome-busy welcome-retained welcome-close-pending)
        set(_studio_case_pack "${_studio_learning_pack_dir}")
        if(_case STREQUAL "welcome-bits2banking" OR _case STREQUAL "guided-bits2banking")
            set(_studio_case_pack "${_studio_bits_learning_fixture}")
        endif()
        add_test(NAME "studio.learning.locations.${_case}"
            COMMAND umicom-studio-learning-locations-test "${_case}" "${_studio_case_pack}")
        set_tests_properties("studio.learning.locations.${_case}" PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;learning;gtk4;ownership;regression")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.learning.locations.${_case}"
                TARGETS umicom-studio-learning-locations-test)
        endif()
    endforeach()
endif()

install(FILES
    "${_studio_learning_pack_dir}/EDITING_FILES.html"
    "${_studio_learning_pack_dir}/READING_BUILD_ERRORS.html"
    "${_studio_learning_pack_dir}/RUNNING_TESTS.html"
    "${_studio_learning_pack_dir}/recent-projects.html"
    "${_studio_learning_pack_dir}/studio-user-guide.html"
    "${_studio_learning_pack_dir}/studio-menu-acceptance.html"
    "${_studio_learning_pack_dir}/bits2banking-courses.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/learning-pack")

# Optional course modules use the same native catalogue as the user guides.
include("${CMAKE_CURRENT_LIST_DIR}/UmicomStudioLearningModules.cmake")

# A second executable directory models a delivered Studio guide. Keeping this
# separate from empty-library cases prevents test order or an application build
# from changing the meaning of the fixture.
if(BUILD_TESTING AND TARGET umicom-studio-learning-locations-test
        AND TARGET Umicom::learning_catalogue_gtk4)
    add_executable(umicom-studio-learning-bundled-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_learning_locations_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-learning-bundled-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include"
        "${_learning_framework_source}/tests")
    target_link_libraries(umicom-studio-learning-bundled-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-learning-bundled-test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/learning-fixtures/bundled/$<CONFIG>")
    umicom_apply_warnings(umicom-studio-learning-bundled-test)
    umicom_apply_sanitizers(umicom-studio-learning-bundled-test)
    include("${_learning_framework_source}/cmake/UmicomLearningModuleBundle.cmake")
    umicom_bundle_learning_module(TARGET umicom-studio-learning-bundled-test
        NAME studio-guide SOURCE_DIR "${_studio_learning_pack_dir}"
        PACK_ID studio.learning-guide PRODUCT studio NO_INSTALL)
    add_test(NAME studio.learning.locations.bundled-guide
        COMMAND umicom-studio-learning-bundled-test bundled-guide "${_studio_learning_pack_dir}")
    set_tests_properties(studio.learning.locations.bundled-guide PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;learning;gtk4;bundle;regression")
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-learning-bundled-test)
    endif()
    if(COMMAND umicom_test_impact_owners)
        umicom_test_impact_owners(studio.learning.locations.bundled-guide
            TARGETS umicom-studio-learning-bundled-test)
    endif()
endif()
