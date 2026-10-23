# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT BUILD_TESTING OR NOT TARGET Umicom::StudioCore)
    return()
endif()
add_executable(umicom-studio-explorer-filter-test tests/test_explorer_filter_transaction.c)
target_link_libraries(umicom-studio-explorer-filter-test PRIVATE Umicom::StudioCore)
set_target_properties(umicom-studio-explorer-filter-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-explorer-filter-test)
umicom_apply_sanitizers(umicom-studio-explorer-filter-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-explorer-filter-test)
endif()
foreach(_case coherent clear long-text long-label invalid-outcome binding unbound allocation-refusal revision-overflow null-strings catalogue-snapshot)
    add_test(NAME studio.explorer_filter_transaction.${_case}
        COMMAND umicom-studio-explorer-filter-test "${_case}")
    set_tests_properties(studio.explorer_filter_transaction.${_case} PROPERTIES
        TIMEOUT 60 LABELS "studio;test-platform;transaction;regression")
endforeach()
