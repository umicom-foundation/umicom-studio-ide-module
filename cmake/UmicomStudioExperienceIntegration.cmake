#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioExperienceIntegration.cmake
#
# PURPOSE:
#   Add the Studio-facing product readiness centre without creating a parallel
#   workbench implementation outside Umicom Framework.
# Created by: Sammy Hegab
# Organisation: Umicom Foundation
# Licence: MIT
#-----------------------------------------------------------------------------

if(NOT TARGET umicom_studio_core)
    message(FATAL_ERROR
        "UmicomStudioExperienceIntegration requires umicom_studio_core")
endif()

target_sources(umicom_studio_core PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/experience_centre.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/experience_projection.c"
)

if(BUILD_TESTING)
    add_executable(umicom-studio-experience-centre-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_experience_centre.c")
    target_link_libraries(umicom-studio-experience-centre-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-experience-centre-test)
    umicom_apply_sanitizers(umicom-studio-experience-centre-test)
    add_test(NAME studio.experience_centre
             COMMAND umicom-studio-experience-centre-test)

    add_executable(umicom-studio-experience-projection-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_experience_projection.c")
    target_link_libraries(umicom-studio-experience-projection-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-experience-projection-test)
    umicom_apply_sanitizers(umicom-studio-experience-projection-test)
    add_test(NAME studio.experience_projection
             COMMAND umicom-studio-experience-projection-test)
endif()
