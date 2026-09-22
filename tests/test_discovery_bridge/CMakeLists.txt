#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: applications/studio/tests/test_discovery_bridge/CMakeLists.txt
# PURPOSE: Compile the actual new bridge include against explicit service seams
#          and real Framework workers/registries; this is not a full Studio build.
# AUTHOR AND ORGANISATION: Sammy Hegab | Umicom Foundation
# LICENCE: MIT
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomStudioDiscoveryBridgeChecks LANGUAGES C)
include(CTest)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
get_filename_component(workspace_root "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(UMICOM_FRAMEWORK_SOURCE_DIR "${workspace_root}/framework" CACHE PATH "Framework source checkout")
add_subdirectory("${UMICOM_FRAMEWORK_SOURCE_DIR}/tests/task_supervision" framework-checks)
add_executable(umicom-studio-discovery-bridge-checks test_bridge.c)
target_include_directories(umicom-studio-discovery-bridge-checks PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../include"
    "${UMICOM_FRAMEWORK_SOURCE_DIR}/tests/task_supervision")
target_link_libraries(umicom-studio-discovery-bridge-checks PRIVATE umicom-background-discovery-core)
if(MSVC)
    target_compile_options(umicom-studio-discovery-bridge-checks PRIVATE /W4 /WX)
else()
    target_compile_options(umicom-studio-discovery-bridge-checks PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
    if(UMICOM_TASK_CHECKS_SANITIZERS)
        target_compile_options(umicom-studio-discovery-bridge-checks PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(umicom-studio-discovery-bridge-checks PRIVATE -fsanitize=address,undefined)
    elseif(UMICOM_TASK_CHECKS_THREAD_SANITIZER)
        target_compile_options(umicom-studio-discovery-bridge-checks PRIVATE -fsanitize=thread)
        target_link_options(umicom-studio-discovery-bridge-checks PRIVATE -fsanitize=thread)
    endif()
endif()
if(BUILD_TESTING)
    set(injected_root "${CMAKE_CURRENT_BINARY_DIR}/injected build")
    file(MAKE_DIRECTORY "${injected_root}")
    file(WRITE "${injected_root}/CTestTestfile.cmake" "# Injected CTest process for Studio bridge checks.\n")
    foreach(scenario success stop generation source build configuration trust view-capacity
            dispose double-arm operation-busy)
        add_test(NAME "studio.discovery_bridge.${scenario}"
            COMMAND umicom-studio-discovery-bridge-checks "${scenario}" "${injected_root}")
        set_tests_properties("studio.discovery_bridge.${scenario}" PROPERTIES TIMEOUT 30)
    endforeach()
endif()
