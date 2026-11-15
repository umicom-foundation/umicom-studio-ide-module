#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioCodeGuardJobs.cmake
#
# PURPOSE:
# Register the thin facade background-job integration checks.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
    add_executable(umicom-studio-codeguard-jobs-test tests/test_codeguard_jobs.c)
    target_link_libraries(umicom-studio-codeguard-jobs-test PRIVATE Umicom::StudioCore)
    target_compile_features(umicom-studio-codeguard-jobs-test PRIVATE c_std_23)
    umicom_apply_warnings(umicom-studio-codeguard-jobs-test)
    umicom_apply_sanitizers(umicom-studio-codeguard-jobs-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-codeguard-jobs-test)
    endif()
    set(_root "${CMAKE_CURRENT_BINARY_DIR}/qualification/codeguard-jobs")
    file(MAKE_DIRECTORY "${_root}/workspace" "${_root}/other")
    file(CONFIGURE OUTPUT "${_root}/workspace/sample.c"
        CONTENT "void sample(void) {\n    malloc(4);\n}\n" NEWLINE_STYLE LF)
    foreach(_case IN ITEMS arguments success empty busy cancelled budget foreign-workspace facade-destroyed repeat)
        add_test(NAME "studio.codeguard_jobs.${_case}"
            COMMAND umicom-studio-codeguard-jobs-test "${_case}" "${_root}")
        set_tests_properties("studio.codeguard_jobs.${_case}" PROPERTIES
            TIMEOUT 25 LABELS "studio;codeguard;jobs;ownership")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners("studio.codeguard_jobs.${_case}" TARGETS umicom-studio-codeguard-jobs-test)
        endif()
    endforeach()
endif()
