# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/../docs/PROCESS_DISCOVERY.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")
if(NOT BUILD_TESTING OR NOT TARGET umicom-studio-workspace-canvas-test OR NOT TARGET umicom-native-attach-adapter-fixture)
    return()
endif()
add_executable(umicom-studio-process-picker-test
    "${CMAKE_CURRENT_LIST_DIR}/../tests/test_process_picker_gtk4.c"
    "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
target_include_directories(umicom-studio-process-picker-test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
target_link_libraries(umicom-studio-process-picker-test PRIVATE
    Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
set_target_properties(umicom-studio-process-picker-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
umicom_apply_warnings(umicom-studio-process-picker-test)
umicom_apply_sanitizers(umicom-studio-process-picker-test)
add_dependencies(umicom-studio-process-picker-test umicom-native-attach-adapter-fixture)
if(COMMAND umicom_register_validation_target)
    umicom_register_validation_target(umicom-studio-process-picker-test)
endif()
foreach(case choose revoke retained manual)
    add_test(NAME studio.debug.process_picker.gtk4.${case} COMMAND umicom-studio-process-picker-test "${case}"
        "$<TARGET_FILE:umicom-native-attach-adapter-fixture>")
    set_tests_properties(studio.debug.process_picker.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
        LABELS "studio;debug;process-picker;gtk4;ownership;fixture")
endforeach()
