#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/check_compiler_centre_output.cmake
#
# PURPOSE:
#   Check complete compiler inspection output and refusal before output for bad inputs.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
if(NOT DEFINED TOOL)
    message(FATAL_ERROR "TOOL is required")
endif()
execute_process(COMMAND "${TOOL}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
    ENCODING UTF-8 TIMEOUT 30)
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT result STREQUAL "0" OR NOT error STREQUAL ""
   OR NOT output MATCHES "^Umicom Compiler Centre\nProviders: 4 \\(4 available\\)\nProfiles: 1\n"
   OR NOT output MATCHES "Active profile: host-debug\n"
   OR NOT output MATCHES "\ngcc[^\n]* selected\n"
   OR output MATCHES "\nclang[^\n]* selected\n")
    message(FATAL_ERROR "Compiler view output is inconsistent: ${result}; ${error}; ${output}")
endif()
string(REPEAT x 4096 overlong)
execute_process(COMMAND "${TOOL}" "${overlong}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
    ENCODING UTF-8 TIMEOUT 30)
if(NOT result STREQUAL "2" OR NOT output STREQUAL "" OR NOT error MATCHES "provider registration")
    message(FATAL_ERROR "Overlong driver was not refused before output")
endif()
execute_process(COMMAND "${TOOL}" gcc clang umicc extra
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
    ENCODING UTF-8 TIMEOUT 30)
if(NOT result STREQUAL "8" OR NOT output STREQUAL "" OR NOT error MATCHES "Usage:")
    message(FATAL_ERROR "Unexpected arguments were not refused before setup")
endif()
