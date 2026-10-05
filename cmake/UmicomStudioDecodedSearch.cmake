# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise the production saved-file search worker, decoding and result activation.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-decoded-search-test tests/test_decoded_search_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-decoded-search-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-decoded-search-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-decoded-search-test)
    umicom_apply_sanitizers(umicom-studio-decoded-search-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-decoded-search-test)
    endif()
    foreach(case utf16-le utf16-be utf8-bom bare-cr unicode-path unsupported draft-preserved stale-index retained-result)
        add_test(NAME studio.search.decoded.gtk4.${case} COMMAND umicom-studio-decoded-search-test ${case})
        set_tests_properties(studio.search.decoded.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;search;gtk4;ownership;regression")
    endforeach()
endif()

