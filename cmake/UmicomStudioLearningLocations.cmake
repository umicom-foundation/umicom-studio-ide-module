# Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
include_guard(GLOBAL)
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-learning-locations-test
        "${CMAKE_CURRENT_LIST_DIR}/../tests/test_learning_locations_gtk4.c"
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/workbench_window.c")
    target_include_directories(umicom-studio-learning-locations-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../src/gui/workbench/include")
    target_link_libraries(umicom-studio-learning-locations-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-learning-locations-test)
    umicom_apply_sanitizers(umicom-studio-learning-locations-test)
    get_target_property(_learning_framework_source umicom_developer SOURCE_DIR)
    foreach(case controls project-separation retained-controls first-lesson relative-without-project
                 document-utf8 document-overflow document-binary)
        add_test(NAME studio.learning.locations.${case} COMMAND umicom-studio-learning-locations-test "${case}" "${_learning_framework_source}")
        set_tests_properties(studio.learning.locations.${case} PROPERTIES TIMEOUT 60
            SKIP_RETURN_CODE 77 LABELS "studio;learning;workspace;gtk4;ownership;regression")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-learning-locations-test)
    endif()
endif()

# Install the user workflows alongside Studio's existing educational guides.
install(FILES
    "${CMAKE_CURRENT_LIST_DIR}/../docs/learning-in-studio.html"
    "${CMAKE_CURRENT_LIST_DIR}/../docs/repository-working-tree-review.html"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/umicom-studio/docs")

# The catalogue is a GUI dependency of workbench consumers, not StudioCore.
# Defer discovery until this directory has created its later acceptance targets;
# this keeps the executable and those tests on the same optional feature path.
function(umicom_studio_link_learning_catalogue)
    if(NOT TARGET Umicom::learning_catalogue_gtk4)
        return()
    endif()
    get_property(_consumers DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_consumer IN LISTS _consumers)
        get_target_property(_sources "${_consumer}" SOURCES)
        foreach(_source IN LISTS _sources)
            if(_source MATCHES "(^|/)workbench_window[.]c$")
                target_link_libraries("${_consumer}" PRIVATE Umicom::learning_catalogue_gtk4)
                target_compile_definitions("${_consumer}" PRIVATE
                    UMICOM_STUDIO_HAS_LEARNING_CATALOGUE=1)
                break()
            endif()
        endforeach()
    endforeach()
endfunction()
cmake_language(DEFER CALL umicom_studio_link_learning_catalogue)
