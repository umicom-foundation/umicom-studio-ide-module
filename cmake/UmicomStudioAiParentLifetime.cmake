# Umicom Studio IDE
# File: cmake/UmicomStudioAiParentLifetime.cmake
#
# PURPOSE:
# Register native AI parent lifetime and callback teardown checks.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT

if(BUILD_TESTING AND TARGET Umicom::StudioCore)
    add_executable(umicom-studio-ai-parent-lifetime-test tests/test_ai_parent_lifetime.c)
    target_link_libraries(umicom-studio-ai-parent-lifetime-test PRIVATE Umicom::StudioCore)
    set_target_properties(umicom-studio-ai-parent-lifetime-test PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    umicom_apply_warnings(umicom-studio-ai-parent-lifetime-test)
    umicom_apply_sanitizers(umicom-studio-ai-parent-lifetime-test)
    if(COMMAND umicom_target_qualification_inputs)
        umicom_target_qualification_inputs(umicom-studio-ai-parent-lifetime-test FILES
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/services.c"
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/services_codeguard.inc"
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/services_lifecycle.inc"
            "${CMAKE_CURRENT_LIST_DIR}/../src/app/build_configurations.inc")
    endif()
    foreach(_direction IN ITEMS apply revert)
        foreach(_phase IN ITEMS read write compensation)
            foreach(_hook IN ITEMS framework-idle platform-idle platform-destroy platform-legacy services-release services-legacy services-stop)
                set(_name "studio.ai.parent-lifetime.patch.${_direction}.${_phase}.${_hook}")
                add_test(NAME "${_name}" COMMAND umicom-studio-ai-parent-lifetime-test patch "${_hook}" "${_phase}" "${_direction}")
                set_tests_properties("${_name}" PROPERTIES TIMEOUT 60 LABELS "studio;ai;lifetime;callback")
                umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-parent-lifetime-test)
            endforeach()
        endforeach()
    endforeach()
    set(_admission_cases null registry-count platform-clean partial-clean)
    set(_provider_cases platform-idle platform-destroy platform-legacy services-release services-legacy services-stop)
    foreach(_group IN ITEMS admission provider)
        foreach(_case IN LISTS _${_group}_cases)
            set(_name "studio.ai.parent-lifetime.${_group}.${_case}")
            add_test(NAME "${_name}" COMMAND umicom-studio-ai-parent-lifetime-test "${_group}" "${_case}")
            set_tests_properties("${_name}" PROPERTIES TIMEOUT 60 LABELS "studio;ai;lifetime;callback")
            umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-parent-lifetime-test)
        endforeach()
    endforeach()

    foreach(_entry IN ITEMS checked legacy)
        foreach(_outcome IN ITEMS success provider-error)
            foreach(_hook IN ITEMS platform-idle platform-destroy platform-legacy services-release services-legacy services-stop)
                set(_name "studio.ai.parent-lifetime.generation.${_entry}.${_outcome}.${_hook}")
                add_test(NAME "${_name}" COMMAND umicom-studio-ai-parent-lifetime-test generation "${_hook}" "${_entry}" "${_outcome}")
                set_tests_properties("${_name}" PROPERTIES TIMEOUT 60 LABELS "studio;ai;lifetime;callback")
                umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-parent-lifetime-test)
            endforeach()
        endforeach()
    endforeach()

    foreach(_entry IN ITEMS legacy scoped)
        foreach(_outcome IN ITEMS success provider-error malformed)
            foreach(_hook IN ITEMS authorengine-idle authorengine-destroy authorengine-legacy
                platform-idle platform-destroy platform-legacy services-release services-legacy services-stop)
                set(_name "studio.ai.parent-lifetime.health.${_entry}.${_outcome}.${_hook}")
                add_test(NAME "${_name}" COMMAND umicom-studio-ai-parent-lifetime-test health "${_hook}" "${_entry}" "${_outcome}")
                set_tests_properties("${_name}" PROPERTIES TIMEOUT 60 LABELS "studio;ai;lifetime;health")
                umicom_test_impact_owners("${_name}" TARGETS umicom-studio-ai-parent-lifetime-test)
            endforeach()
        endforeach()
    endforeach()
endif()
