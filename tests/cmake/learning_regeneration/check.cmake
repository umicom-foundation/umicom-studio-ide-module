#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: tests/cmake/learning_regeneration/check.cmake
#
# PURPOSE:
#   Verify production learning manifest stability, content regeneration and staging without full application dependencies.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

file(MAKE_DIRECTORY "${WORK}")
set(_configure "${CMAKE_COMMAND}" -S "${FIXTURE}" -B "${WORK}"
    -G "${GENERATOR}" "-DFRAMEWORK_ROOT=${FRAMEWORK_ROOT}"
    "-DSTUDIO_ROOT=${STUDIO_ROOT}" "-DCMAKE_C_COMPILER=${C_COMPILER}")
if(MAKE_PROGRAM)
    list(APPEND _configure "-DCMAKE_MAKE_PROGRAM=${MAKE_PROGRAM}")
endif()
function(Run label)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result
        OUTPUT_VARIABLE _out ERROR_VARIABLE _err TIMEOUT 35)
    file(WRITE "${WORK}/${label}.log" "${_out}\n${_err}")
    if(NOT "${_result}" STREQUAL "0")
        message(FATAL_ERROR "${label} failed (${_result}):\n${_out}\n${_err}")
    endif()
    set(RUN_TEXT "${_out}\n${_err}" PARENT_SCOPE)
endfunction()
Run(initial ${_configure})
set(_manifest "${WORK}/framework-wiki-pack/learning-pack.json")
file(TIMESTAMP "${_manifest}" _first "%s" UTC)
file(SHA256 "${_manifest}" _first_hash)
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1.1)
Run(repeated ${_configure})
file(TIMESTAMP "${_manifest}" _second "%s" UTC)
file(SHA256 "${_manifest}" _second_hash)
if(NOT _first_hash STREQUAL _second_hash)
    message(FATAL_ERROR "Unchanged inputs changed the manifest content")
endif()
if(NOT _first STREQUAL _second)
    message(FATAL_ERROR "Unchanged manifest was rewritten: ${_first} -> ${_second}; this watched configure input can cause a Ninja regeneration loop")
endif()
# The echo target must return promptly. It exercises Ninja regeneration without
# compiling Studio or its graphical dependencies.
Run(settled "${CMAKE_COMMAND}" --build "${WORK}" --target check-noop)
if(RUN_TEXT MATCHES "Re-running CMake")
    message(FATAL_ERROR "Unchanged configure inputs still triggered regeneration:\n${RUN_TEXT}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1.1)
file(APPEND "${WORK}/proxy/docs/learning/studio-wiki/index.html" "<p>A reviewed lesson update.</p>\n")
Run(content-change "${CMAKE_COMMAND}" --build "${WORK}" --target check-noop)
file(SHA256 "${_manifest}" _changed_hash)
if(_changed_hash STREQUAL _first_hash)
    message(FATAL_ERROR "A real lesson change was not reflected in the generated manifest")
endif()
file(READ "${_manifest}" _json)
string(JSON _declared GET "${_json}" resources 0 sha256)
file(SHA256 "${WORK}/framework-wiki-pack/index.html" _actual)
if(NOT _declared STREQUAL _actual)
    message(FATAL_ERROR "Updated page checksum was not preserved")
endif()
Run(after-change "${CMAKE_COMMAND}" --build "${WORK}" --target check-noop)
if(RUN_TEXT MATCHES "Re-running CMake")
    message(FATAL_ERROR "Regeneration did not settle after the lesson update")
endif()
Run(staging "${CMAKE_COMMAND}" --build "${WORK}" --target umicom-studio-ide-module-framework-wiki)
file(SHA256 "${WORK}/bin/learning-modules/framework-wiki/learning-pack.json" _staged)
if(NOT _staged STREQUAL _changed_hash)
    message(FATAL_ERROR "Staging did not retain the updated manifest")
endif()
message(STATUS "Unchanged manifests keep timestamps; real lesson changes regenerate and stage correctly")
