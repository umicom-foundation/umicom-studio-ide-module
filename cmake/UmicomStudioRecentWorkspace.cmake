#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioRecentWorkspace.cmake
#
# PURPOSE:
#   Register native Welcome history actions against the real service container.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-recent-workspace-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_recent_workspace_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-recent-workspace-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-recent-workspace-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-recent-workspace-test)
    umicom_apply_sanitizers(umicom-studio-recent-workspace-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-recent-workspace-test)
    endif()
    foreach(_case IN ITEMS remove pinned missing-folder stale unrelated refresh retained
            damaged save-failure retry cwd refresh-availability
            pin unpin pin-stale pin-save-failure pin-retry pin-cwd pin-missing-folder
            pin-other-changed pin-retained)
        add_test(NAME studio.welcome.recent.${_case}
            COMMAND umicom-studio-recent-workspace-test "${_case}")
        set_tests_properties(studio.welcome.recent.${_case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;welcome;recent-items;persistence;gtk4;regression")
    endforeach()
endif()

install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/recent-projects.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")

# These service cases do not require a display; graphical skips cannot replace them.
if(BUILD_TESTING)
    add_executable(umicom-studio-recent-workspace-service-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_recent_workspace_service.c")
    target_link_libraries(umicom-studio-recent-workspace-service-test PRIVATE
        Umicom::StudioCore PkgConfig::GLIB)
    umicom_apply_warnings(umicom-studio-recent-workspace-service-test)
    umicom_apply_sanitizers(umicom-studio-recent-workspace-service-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-recent-workspace-service-test)
    endif()
    foreach(_case IN ITEMS remove cwd damaged wrong-kind missing null open-workspace
            pin-set pin-unset pin-cwd pin-damaged pin-wrong-kind pin-missing pin-null
            pin-open-workspace pin-stale)
        add_test(NAME studio.services.recent.${_case}
            COMMAND umicom-studio-recent-workspace-service-test "${_case}")
        set_tests_properties(studio.services.recent.${_case} PROPERTIES
            TIMEOUT 60 LABELS "studio;services;recent-items;persistence;regression")
    endforeach()
    if(COMMAND umicom_target_qualification_inputs)
        umicom_target_qualification_inputs(umicom-studio-recent-workspace-service-test FILES
            "${CMAKE_CURRENT_LIST_DIR}/../tests/studio_test_state.h")
        if(TARGET umicom-studio-services-test)
            umicom_target_qualification_inputs(umicom-studio-services-test FILES
                "${CMAKE_CURRENT_LIST_DIR}/../tests/studio_test_state.h")
        endif()
    endif()
endif()
