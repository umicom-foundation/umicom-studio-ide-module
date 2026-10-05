# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Exercise searchable editor action groups through the production editor toolbar.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-action-groups-test tests/test_editor_action_groups_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-action-groups-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-action-groups-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-action-groups-test)
    umicom_apply_sanitizers(umicom-studio-action-groups-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-action-groups-test)
    endif()
    foreach(case discovery code-filter navigation-filter clear-filter matching-action snippet-action sensitivity retained-filter caption)
        add_test(NAME studio.editor.action_groups.gtk4.${case} COMMAND umicom-studio-action-groups-test ${case})
        set_tests_properties(studio.editor.action_groups.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;editor;action-menu;gtk4;ownership;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/EDITOR_TOOLS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)
