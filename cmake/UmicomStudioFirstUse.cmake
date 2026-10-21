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
        project-create-hidden project-create-existing close-during-create workspace-refresh-edit
        project-prompt-retired project-prompt-retained-window
        wizard-edit-settings wizard-hidden-editor wizard-close-during-adoption project-open-failure-draft
        settings-present-complete settings-present-close settings-present-replace
        problem-next-stale problem-previous-stale
        keyboard-new keyboard-save keyboard-modifiers keyboard-retired
        search-keyboard search-options search-retired search-feedback-close search-location
        run-untrusted run-unconfigured run-missing run-working-folder run-feedback-close
        replace-keyboard replace-options replace-retired replace-feedback-close
        close-dialog-cancel close-dialog-confirm close-dialog-retained-dialog
        close-dialog-retained-button close-dialog-retired-owner close-dialog-reparented-cancel
        batch-save batch-save-start-close batch-save-completion-close batch-save-cancel
        batch-close batch-close-start-close batch-close-completion-close batch-close-cancel
        history-reopen-hidden history-reopen-missing history-reopen-focus-close
        history-back-hidden history-forward-hidden
        button-new button-open button-retired-command button-retired-argument
        button-retired-palette button-retired-layout
        build-denied-closed build-denied-untrusted build-denied-other-project build-denied-feedback-close
        navigation-document-next navigation-document-previous navigation-bookmark-next
        navigation-bookmark-previous navigation-bookmark-empty navigation-bookmark-focus-close
        location-direct-hidden location-invalid location-prompt-hidden location-prompt-cancel location-focus-close
        reload-menu reload-toolbar reload-canonical reload-cancel reload-disk-changed reload-focus-close reload-undo external-changed external-missing external-feedback-close
        form-project-open form-application-open form-project-create-close form-application-create-close form-project-create-reenter form-application-create-reenter form-project-present-close form-application-present-close form-project-present-reenter form-application-present-reenter form-project-replace-close form-application-replace-close
        line-hidden line-keyboard line-argument line-focus-close line-toolbar
        evidence-selected-close evidence-results-close evidence-coverage-close evidence-output-close evidence-locations-close evidence-row-close evidence-selection evidence-invalid-bytes evidence-clipboard-close evidence-reenter evidence-reselect
        explorer-row-close explorer-summary-close explorer-status-close explorer-filter-during-refresh explorer-workspace-during-refresh explorer-reenter explorer-unchanged explorer-same-root-reopen explorer-retained-old-row)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        add_test(NAME studio.first_use.gtk4.${case} COMMAND umicom-studio-first-use-test ${case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/first-use-${case}")
        set_tests_properties(studio.first_use.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;document;first-use;acceptance")
    endforeach()
    # These cases use the generated C23 project and installed native toolchain.
    # Missing compilers are failures with captured output, not passing skips.
    foreach(case real-build-run real-build-repair discovery-relative discovery-absolute discovery-run-relative
        discovery-feedback-close discovery-run-feedback-close)
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
        debug-open debug-close debug-profile debug-rebind debug-sync-open relative-binding missing-preserves-watch
        refresh-closed refresh-new-files refresh-close prepare-discovery-failure sync-discovery-failure test-root-owner test-child-open)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/interactive-workspace-${case}")
        add_test(NAME studio.workspace.interactive.${case}
            COMMAND umicom-studio-interactive-workspace-test ${case})
        set_tests_properties(studio.workspace.interactive.${case} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/interactive-workspace-${case}"
            TIMEOUT 60 LABELS "studio;workspace;first-use;regression")
    endforeach()
endif()

# The registered command must enforce project admission without a GTK caller.
if(BUILD_TESTING)
    add_executable(umicom-studio-discovery-admission-test tests/test_discovery_admission.c)
    target_link_libraries(umicom-studio-discovery-admission-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-discovery-admission-test)
    umicom_apply_sanitizers(umicom-studio-discovery-admission-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-discovery-admission-test)
    endif()
    foreach(case closed untrusted armed-untrusted profile-mismatch catalogue-mismatch empty-owner)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/discovery-admission-${case}")
        add_test(NAME studio.tests.admission.${case} COMMAND umicom-studio-discovery-admission-test ${case})
        set_tests_properties(studio.tests.admission.${case} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/discovery-admission-${case}"
            TIMEOUT 60 LABELS "studio;tests;first-use;regression")
    endforeach()
endif()

# Keep trust and project ownership at the command boundary for every frontend.
if(BUILD_TESTING)
    add_executable(umicom-studio-build-command-admission-test tests/test_build_command_admission.c)
    target_link_libraries(umicom-studio-build-command-admission-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-build-command-admission-test)
    umicom_apply_sanitizers(umicom-studio-build-command-admission-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-build-command-admission-test)
    endif()
    foreach(operation configure compile test clean run run-current install package rebuild deploy retry run-next run-all debug
        tests-all tests-selected tests-debug tests-coverage tests-failed tests-repeat)
        foreach(state closed untrusted other-project relative-source)
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/build-admission-${operation}-${state}")
            add_test(NAME studio.build.admission.${operation}.${state}
                COMMAND umicom-studio-build-command-admission-test ${operation} ${state})
            set_tests_properties(studio.build.admission.${operation}.${state} PROPERTIES
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/build-admission-${operation}-${state}"
                TIMEOUT 60 LABELS "studio;build;debug;first-use;regression")
        endforeach()
    endforeach()
endif()

# Preserve retained catalogue evidence while refusing execution from another origin.
if(BUILD_TESTING)
    add_executable(umicom-studio-test-command-origin-test tests/test_test_command_origin.c)
    target_link_libraries(umicom-studio-test-command-origin-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-test-command-origin-test)
    umicom_apply_sanitizers(umicom-studio-test-command-origin-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-test-command-origin-test)
    endif()
    foreach(case other-build other-config other-project mixed-projects missing-origin wrong-explorer)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/test-command-origin-${case}")
        add_test(NAME studio.tests.command-origin.${case} COMMAND umicom-studio-test-command-origin-test ${case})
        set_tests_properties(studio.tests.command-origin.${case} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/test-command-origin-${case}"
            TIMEOUT 60 LABELS "studio;tests;first-use;regression")
    endforeach()
endif()


# Deterministic native chooser replies exercise the production workbench unity.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-document-chooser-test tests/test_document_chooser_gtk4.c)
    target_include_directories(umicom-studio-document-chooser-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-document-chooser-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-document-chooser-test)
    umicom_apply_sanitizers(umicom-studio-document-chooser-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-chooser-test)
    endif()
    foreach(case open-create-close save-create-close folder-create-close
        open-seed-close save-seed-close folder-seed-close open-present-close save-present-close folder-present-close
        open-cancel save-cancel folder-cancel open-choose save-choose folder-choose
        open-late-reply save-late-reply folder-late-reply save-other-tab open-switch-project)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/chooser-${case}")
        add_test(NAME studio.document_chooser.gtk4.${case} COMMAND umicom-studio-document-chooser-test ${case}
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/chooser-${case}")
        set_tests_properties(studio.document_chooser.gtk4.${case} PROPERTIES TIMEOUT 60
            SKIP_RETURN_CODE 77 LABELS "studio;gtk4;document;chooser;regression")
    endforeach()
endif()
