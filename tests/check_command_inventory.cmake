#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/check_command_inventory.cmake
#
# PURPOSE:
#   Verify native command discovery and filtering without executing commands.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
if(NOT DEFINED PROGRAM OR NOT EXISTS "${PROGRAM}")
    message(FATAL_ERROR "Supply the built Studio state executable")
endif()
if(CASE STREQUAL "all")
    set(prefix "")
    set(id_pattern "[^\t\n]+")
elseif(CASE STREQUAL "studio")
    set(prefix "studio.")
    set(id_pattern "studio[.][^\t\n]+")
elseif(CASE STREQUAL "empty")
    set(prefix "__umicom_inventory_no_match__.")
    set(id_pattern "[^\t\n]+")
else()
    message(FATAL_ERROR "Unknown command discovery case")
endif()
execute_process(COMMAND "${PROGRAM}" --commands "${prefix}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 90)
if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Studio command discovery failed (${result})\n${output}\n${errors}")
endif()
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT output MATCHES "^Matching commands: ([0-9]+)\nCommand\tTitle\tCategory\tFlags\n")
    message(FATAL_ERROR "Missing command inventory heading\n${output}")
endif()
set(expected "${CMAKE_MATCH_1}")
if(CASE STREQUAL "empty")
    if(NOT expected EQUAL 0)
        message(FATAL_ERROR "An unknown namespace returned ${expected} commands")
    endif()
elseif(expected LESS 1)
    message(FATAL_ERROR "Started Studio did not publish the requested commands")
endif()
string(REGEX REPLACE "^Matching commands: [0-9]+\nCommand\tTitle\tCategory\tFlags\n" "" rows "${output}")
# Count physical rows without converting metadata containing semicolons to a
# CMake list. Titles and categories may be empty but must remain one table cell.
string(REGEX REPLACE "[^\n]+\n" "x" row_markers "${rows}")
string(LENGTH "${row_markers}" actual)
if(NOT actual EQUAL expected)
    message(FATAL_ERROR "Command count ${expected} disagrees with ${actual} rows\n${output}")
endif()
if(NOT rows STREQUAL "")
    if(NOT rows MATCHES "^(${id_pattern}\t[^\t\n]*\t[^\t\n]*\t0x[0-9a-f]+\n)+$")
        message(FATAL_ERROR "Malformed or unfiltered command metadata\n${output}")
    endif()
endif()
