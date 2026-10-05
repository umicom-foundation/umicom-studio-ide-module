# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise folding discovery and native collapse using a deterministic language peer.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test AND TARGET umicom-language-process-fixture)
    add_executable(umicom-studio-syntax-folding-test tests/test_syntax_folding_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-syntax-folding-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-syntax-folding-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-syntax-folding-test)
    umicom_apply_sanitizers(umicom-studio-syntax-folding-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-syntax-folding-test)
    endif()
    add_dependencies(umicom-studio-syntax-folding-test umicom-language-process-fixture)
    if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_options(umicom-studio-syntax-folding-test PRIVATE -municode)
    endif()
    foreach(case open cancel retained empty parent-close collapse reveal draft-history readonly single-line invalid-response)
        add_test(NAME studio.language.syntax_folding.gtk4.${case} COMMAND umicom-studio-syntax-folding-test ${case} "$<TARGET_FILE:umicom-language-process-fixture>")
        set_tests_properties(studio.language.syntax_folding.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;syntax-folding;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/SYNTAX_FOLDING.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
