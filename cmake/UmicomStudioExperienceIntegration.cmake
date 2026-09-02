#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioExperienceIntegration.cmake
#
# PURPOSE:
#   Add the Studio-facing product readiness centre without creating a parallel
#   workbench implementation outside Umicom Framework.
# AUTHOR AND ORGANISATION:
#   Sammy Hegab
#   Umicom Foundation
#
# LICENCE:
#   MIT
#-----------------------------------------------------------------------------

if(NOT TARGET umicom_studio_core)
    message(FATAL_ERROR
        "UmicomStudioExperienceIntegration requires umicom_studio_core")
endif()

target_sources(umicom_studio_core PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/experience_centre.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/experience_projection.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/application_runtime_centre.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/application_panel_centre.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/app/application_feature_centre.c"
)

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
    add_executable(umicom-studio-experience-centre-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_experience_centre.c")
    target_link_libraries(umicom-studio-experience-centre-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-experience-centre-test)
    umicom_apply_sanitizers(umicom-studio-experience-centre-test)
    add_test(NAME studio.experience_centre
             COMMAND umicom-studio-experience-centre-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-experience-centre-test)
    endif()

    add_executable(umicom-studio-experience-projection-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_experience_projection.c")
    target_link_libraries(umicom-studio-experience-projection-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-experience-projection-test)
    umicom_apply_sanitizers(umicom-studio-experience-projection-test)
    add_test(NAME studio.experience_projection
             COMMAND umicom-studio-experience-projection-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-experience-projection-test)
    endif()

    add_executable(umicom-studio-application-runtime-centre-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_application_runtime_centre.c")
    target_link_libraries(umicom-studio-application-runtime-centre-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-application-runtime-centre-test)
    umicom_apply_sanitizers(umicom-studio-application-runtime-centre-test)
    add_test(NAME studio.application_runtime_centre
             COMMAND umicom-studio-application-runtime-centre-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-application-runtime-centre-test)
    endif()

    add_executable(umicom-studio-application-panel-centre-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_application_panel_centre.c")
    target_link_libraries(umicom-studio-application-panel-centre-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-application-panel-centre-test)
    umicom_apply_sanitizers(umicom-studio-application-panel-centre-test)
    add_test(NAME studio.application_panel_centre
             COMMAND umicom-studio-application-panel-centre-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-application-panel-centre-test)
    endif()

    add_executable(umicom-studio-application-feature-centre-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_application_feature_centre.c")
    target_link_libraries(umicom-studio-application-feature-centre-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-application-feature-centre-test)
    umicom_apply_sanitizers(umicom-studio-application-feature-centre-test)
    add_test(NAME studio.application_feature_centre
             COMMAND umicom-studio-application-feature-centre-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-application-feature-centre-test)
    endif()

endif()
