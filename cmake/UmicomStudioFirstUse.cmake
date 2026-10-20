# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
# Use the actual workbench so model-only success cannot hide unreachable editors.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-first-use-test
        tests/test_first_use_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-first-use-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-first-use-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-first-use-test)
    umicom_apply_sanitizers(umicom-studio-first-use-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-first-use-test)
    endif()
    foreach(case new-file hidden-new open-edit-save hidden-open unicode-path missing-file reopen-dirty project-folder tab-switch build-guidance close-during-open build-unsaved repository-switch close-during-feedback debug-unsaved debug-canonical-unsaved
        project-create-hidden project-create-existing close-during-create workspace-refresh-edit)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        add_test(NAME studio.first_use.gtk4.${case} COMMAND umicom-studio-first-use-test ${case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        set_tests_properties(studio.first_use.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;document;first-use;acceptance")
    endforeach()
    # These cases use the generated C23 project and installed native toolchain.
    # Missing compilers are failures with captured output, not passing skips.
    foreach(case real-build-run real-build-repair)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        add_test(NAME studio.first_use.gtk4.${case} COMMAND umicom-studio-first-use-test ${case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        set_tests_properties(studio.first_use.gtk4.${case} PROPERTIES
            TIMEOUT 600 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;first-use;acceptance;toolchain")
    endforeach()
endif()

# Keep service-level opening checks usable without a graphical display.
if(BUILD_TESTING)
    add_executable(umicom-studio-interactive-workspace-test tests/test_interactive_workspace.c)
    target_link_libraries(umicom-studio-interactive-workspace-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-interactive-workspace-test)
    umicom_apply_sanitizers(umicom-studio-interactive-workspace-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-interactive-workspace-test)
    endif()
    foreach(case prepare scan saved-profile missing-folder close-scan sync-compatibility repository-binding repository-close
        debug-open debug-close debug-profile debug-sync-open relative-binding missing-preserves-watch
        refresh-closed refresh-new-files refresh-close)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/interactive-workspace-${case}")
        add_test(NAME studio.workspace.interactive.${case}
            COMMAND umicom-studio-interactive-workspace-test ${case})
        set_tests_properties(studio.workspace.interactive.${case} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/interactive-workspace-${case}"
            TIMEOUT 60 LABELS "studio;workspace;first-use;regression")
    endforeach()
endif()
