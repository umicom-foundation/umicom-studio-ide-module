# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/LIVE_SOURCE_DIAGNOSTICS.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test OR
   NOT TARGET umicom-live-diagnostic-server-fixture)
    return()
endif()
if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    return()
endif()
add_executable(umicom-studio-live-diagnostics-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_live_diagnostics_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umicom-studio-live-diagnostics-test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umicom-studio-live-diagnostics-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-live-diagnostics-test PROPERTIES
    C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
add_dependencies(umicom-studio-live-diagnostics-test umicom-live-diagnostic-server-fixture)
umicom_apply_warnings(umicom-studio-live-diagnostics-test)
umicom_apply_sanitizers(umicom-studio-live-diagnostics-test)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-live-diagnostics-test)
endif()
foreach(case update retained revoke project-close parent-close denied navigate navigate-stale)
    add_test(NAME studio.live_diagnostics.gtk4.${case}
        COMMAND umicom-studio-live-diagnostics-test "${case}" "$<TARGET_FILE:umicom-live-diagnostic-server-fixture>")
    set_tests_properties(studio.live_diagnostics.gtk4.${case} PROPERTIES
        TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;diagnostics;gtk4;ownership;acceptance")
endforeach()
