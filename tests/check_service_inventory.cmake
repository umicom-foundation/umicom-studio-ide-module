#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/check_service_inventory.cmake
#
# PURPOSE:
#   Verify the native service listing reports coherent copied metadata.
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
execute_process(COMMAND "${PROGRAM}" --services
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 90)
if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Studio service discovery failed (${result})\n${output}\n${errors}")
endif()
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT output MATCHES "^Registered services: ([0-9]+)\nService\tProvider\tVersion\tFlags\n")
    message(FATAL_ERROR "Missing service inventory heading\n${output}")
endif()
set(expected "${CMAKE_MATCH_1}")
if(expected LESS 1)
    message(FATAL_ERROR "Started Studio did not publish any services")
endif()
string(REGEX REPLACE "^Registered services: [0-9]+\nService\tProvider\tVersion\tFlags\n" "" rows "${output}")
string(REGEX MATCHALL "[^\n]+\n" entries "${rows}")
list(LENGTH entries actual)
if(NOT actual EQUAL expected)
    message(FATAL_ERROR "Service count ${expected} disagrees with ${actual} rows\n${output}")
endif()
foreach(row IN LISTS entries)
    if(NOT row MATCHES "^[^\t\n]+\t[^\t\n]+\t[0-9]+\\.[0-9]+\\.[0-9]+\t0x[0-9a-f]+\n$")
        message(FATAL_ERROR "Malformed service metadata row: ${row}")
    endif()
endforeach()
