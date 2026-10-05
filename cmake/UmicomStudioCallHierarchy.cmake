# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise both caller and callee actions without starting a language server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-call-hierarchy-test tests/test_call_hierarchy_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-call-hierarchy-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-call-hierarchy-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-call-hierarchy-test)
    umicom_apply_sanitizers(umicom-studio-call-hierarchy-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-call-hierarchy-test)
    endif()
    foreach(case callers-open callers-cancel callers-retained callers-empty callers-parent-close callees-open callees-cancel callees-retained callees-empty callees-parent-close)
        add_test(NAME studio.language.call_hierarchy.gtk4.${case} COMMAND umicom-studio-call-hierarchy-test ${case})
        set_tests_properties(studio.language.call_hierarchy.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;call-hierarchy;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/CALL_HIERARCHY.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
