#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/check_polyglot_command_json.cmake
#
# PURPOSE:
#   Check complete compiler-preview JSON, explicit directory metadata and argument refusal.
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
set(directory "reviewed project/\"quoted\"")
execute_process(COMMAND "${TOOL}" --directory "${directory}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
    ENCODING UTF-8 TIMEOUT 30)
if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
    message(FATAL_ERROR "Preview failed: ${result}; ${error}")
endif()
string(JSON rows ERROR_VARIABLE parse_error LENGTH "${output}")
if(NOT parse_error STREQUAL "NOTFOUND" OR NOT rows EQUAL 1)
    message(FATAL_ERROR "Expected one complete compilation command: ${parse_error}")
endif()
string(JSON actual_directory GET "${output}" 0 directory)
string(JSON source GET "${output}" 0 file)
string(JSON object GET "${output}" 0 output)
string(JSON executable GET "${output}" 0 arguments 0)
if(NOT actual_directory STREQUAL directory OR NOT source STREQUAL "src/main.c"
   OR NOT object STREQUAL "obj/main.o" OR NOT executable STREQUAL "gcc")
    message(FATAL_ERROR "Preview metadata does not match the requested example")
endif()
execute_process(COMMAND "${TOOL}" --unknown
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
    ENCODING UTF-8 TIMEOUT 30)
if(NOT result STREQUAL "2" OR NOT output STREQUAL "" OR NOT error MATCHES "Usage:")
    message(FATAL_ERROR "Unknown argument was not refused before preview")
endif()
