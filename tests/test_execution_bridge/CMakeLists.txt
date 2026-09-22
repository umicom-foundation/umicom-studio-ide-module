#-----------------------------------------------------------------------------
# Umicom Studio IDE | Sammy Hegab | Umicom Foundation | MIT
# File: applications/studio/tests/test_execution_bridge/CMakeLists.txt
# Exercise owner-thread publication with Framework's real worker and registries.
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomStudioTestRunChecks LANGUAGES C)
include(CTest)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
get_filename_component(workspace_root "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(UMICOM_FRAMEWORK_SOURCE_DIR "${workspace_root}/framework" CACHE PATH "Framework source checkout")
add_subdirectory("${UMICOM_FRAMEWORK_SOURCE_DIR}/tests/ctest_jobs" framework-checks)
add_library(umicom-studio-run-records STATIC
    "${UMICOM_FRAMEWORK_SOURCE_DIR}/src/test_platform/result.c"
    "${UMICOM_FRAMEWORK_SOURCE_DIR}/src/test_platform/output.c")
target_include_directories(umicom-studio-run-records PUBLIC "${UMICOM_FRAMEWORK_SOURCE_DIR}/include")
add_executable(umicom-studio-test-run-checks test_bridge.c)
target_include_directories(umicom-studio-test-run-checks PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../include")
target_link_libraries(umicom-studio-test-run-checks PRIVATE umicom-studio-run-records
    umicom-ctest-json-core umicom-ctest-checks-core umicom-ctest-checks-process)
foreach(target umicom-studio-test-run-checks umicom-studio-run-records)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
        if(UMICOM_CTEST_JOBS_SANITIZERS)
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        elseif(UMICOM_CTEST_JOBS_THREAD_SANITIZER)
            target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=thread)
        endif()
    endif()
endforeach()
if(BUILD_TESTING)
    foreach(case success repeat disabled skipped failure stop-failure partial-stop
            queued-stop double-arm busy-operation invalid-context copied-context
            wrong-root wrong-configuration missing-item history-full output-full
            sequence-overflow existing-output generation source configuration trust
            catalogue history output discovery late-stop retained-result dispose
            publish-refused invalid-plan publication-budget invalid-kind stale-selection)
        add_test(NAME "studio.test_run.${case}" COMMAND umicom-studio-test-run-checks
            "${case}" "${CMAKE_CURRENT_BINARY_DIR}/framework-checks/Notes A $<CONFIG>")
        set_tests_properties("studio.test_run.${case}" PROPERTIES TIMEOUT 30
            FIXTURES_REQUIRED "ctest_jobs_A" RESOURCE_LOCK ctest_jobs_fixture)
    endforeach()
endif()
