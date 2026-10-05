# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise both editor actions without starting a language server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-type-navigation-test tests/test_type_navigation_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-type-navigation-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-type-navigation-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-type-navigation-test)
    umicom_apply_sanitizers(umicom-studio-type-navigation-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-type-navigation-test)
    endif()
    foreach(case type-open type-cancel type-retained type-empty type-parent-close implementation-open implementation-cancel implementation-retained implementation-empty implementation-parent-close)
        add_test(NAME studio.language.type_navigation.gtk4.${case} COMMAND umicom-studio-type-navigation-test ${case})
        set_tests_properties(studio.language.type_navigation.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;type-navigation;gtk4;ownership;regression")
    endforeach()
endif()


install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/TYPE_NAVIGATION.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
