# Umicom Studio IDE
# File: cmake/UmicomStudioAiContextPreview.cmake
#
# PURPOSE:
# Register native Framework-backed Studio context preview scenarios.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT

if(BUILD_TESTING AND TARGET Umicom::StudioCore)
    add_executable(umicom-studio-ai-context-preview-test tests/test_ai_context_preview.c)
    target_link_libraries(umicom-studio-ai-context-preview-test PRIVATE Umicom::StudioCore)
    set_target_properties(umicom-studio-ai-context-preview-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-ai-context-preview-test)
    umicom_apply_sanitizers(umicom-studio-ai-context-preview-test)
    foreach(_case IN ITEMS preview
            context-stale
            policy-stale
            runtime-stale
            missing-runtime
            null-platform
            null-plan
            null-observation
            check-null-platform
            check-null-observation
            refresh)
        set(_name "studio.ai.context-preview.${_case}")
        add_test(NAME "${_name}" COMMAND umicom-studio-ai-context-preview-test "${_case}")
        set_tests_properties("${_name}" PROPERTIES TIMEOUT 45 LABELS "studio;ai;context;observation")
        umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-context-preview-test)
    endforeach()
endif()
