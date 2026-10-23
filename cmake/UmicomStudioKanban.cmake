# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(NOT TARGET Umicom::kanban_gtk4)
    return()
endif()
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-kanban-entry-test
        tests/test_kanban_entry_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-kanban-entry-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-kanban-entry-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    set_target_properties(umicom-studio-kanban-entry-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-kanban-entry-test)
    umicom_apply_sanitizers(umicom-studio-kanban-entry-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-kanban-entry-test)
    endif()
    foreach(_case IN ITEMS entry retained)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/kanban-${_case}")
        add_test(NAME studio.kanban.gtk4.${_case} COMMAND umicom-studio-kanban-entry-test "${_case}"
            "${CMAKE_CURRENT_BINARY_DIR}/qualification/kanban-${_case}")
        set_tests_properties(studio.kanban.gtk4.${_case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;kanban;gtk4;ownership;acceptance")
        if(COMMAND umicom_test_impact_owners)
            umicom_test_impact_owners(studio.kanban.gtk4.${_case} TARGETS umicom-studio-kanban-entry-test)
        endif()
    endforeach()
endif()
# Apply the adapter consistently to every actual workbench consumer, including
# acceptance tests. Headless StudioCore does not acquire a GTK dependency.
get_property(_studio_kanban_consumers DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(_consumer IN LISTS _studio_kanban_consumers)
    get_target_property(_sources "${_consumer}" SOURCES)
    foreach(_source IN LISTS _sources)
        if(_source MATCHES "(^|/)workbench_window[.]c$")
            target_link_libraries("${_consumer}" PRIVATE Umicom::kanban_gtk4)
            target_compile_definitions("${_consumer}" PRIVATE UMICOM_STUDIO_HAS_KANBAN=1)
            break()
        endif()
    endforeach()
endforeach()
