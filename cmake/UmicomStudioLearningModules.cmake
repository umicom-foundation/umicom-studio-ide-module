#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/UmicomStudioLearningModules.cmake
#
# PURPOSE:
#   Compose optional course content and the Framework wiki as Studio content modules.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#---------------------------------------------------------------------------

include_guard(GLOBAL)
if(NOT TARGET umicom-studio-ide OR NOT TARGET Umicom::learning_catalogue_gtk4)
    return()
endif()
get_target_property(_module_framework umicom_developer SOURCE_DIR)
include("${_module_framework}/cmake/UmicomLearningModuleBundle.cmake")
umicom_bundle_learning_module(TARGET umicom-studio-ide NAME studio-guide
    SOURCE_DIR "${_studio_learning_pack_dir}" PACK_ID studio.learning-guide PRODUCT studio)

# The optional content source is chosen at configure time and copied into the
# distribution. The running application never searches a developer's checkout.
get_filename_component(_bits_default "${_module_framework}/../../Umicom-Bits2Banking/content/courses/framework-and-finance" ABSOLUTE)
set(UMICOM_STUDIO_BITS2BANKING_MODULE "${_bits_default}" CACHE PATH
    "Bits2Banking course collection containing learning-pack.json; empty omits bundled courses")
if(UMICOM_STUDIO_BITS2BANKING_MODULE)
    if(EXISTS "${UMICOM_STUDIO_BITS2BANKING_MODULE}/learning-pack.json")
        umicom_bundle_learning_module(TARGET umicom-studio-ide NAME bits2banking
            SOURCE_DIR "${UMICOM_STUDIO_BITS2BANKING_MODULE}"
            PACK_ID bits2banking.framework-finance PRODUCT bits2banking EXTRA_FILES course.css)
    else()
        message(STATUS "Bits2Banking is not bundled. Set UMICOM_STUDIO_BITS2BANKING_MODULE to the course collection to include it.")
    endif()
endif()
set(_wiki_source "${_module_framework}/docs/learning/studio-wiki")
set(_wiki_pack "${CMAKE_CURRENT_BINARY_DIR}/framework-wiki-pack")
file(MAKE_DIRECTORY "${_wiki_pack}")
set(_wiki_title_index "Introduction to Umicom Framework")
set(_wiki_title_first-program "Your first C program")
set(_wiki_title_framework-tour "How the Framework fits an application")
set(_wiki_title_gtk-window "Connect a window to shared behaviour")
set(_wiki_title_build-workflow "Configure, build, test and run")
set(_wiki_title_extending-studio "Extend Studio through shared contracts")
set(_wiki_title_module-authoring "Create a learning content module")
set(_wiki_title_source-reference "Find the implementation")
set(_wiki_title_glossary "Words used in this wiki")
set(_wiki_resources "")
foreach(_page index first-program gtk-window framework-tour build-workflow extending-studio module-authoring source-reference glossary)
    configure_file("${_wiki_source}/${_page}.html" "${_wiki_pack}/${_page}.html" COPYONLY)
    file(SHA256 "${_wiki_pack}/${_page}.html" _hash)
    if(_wiki_resources)
        string(APPEND _wiki_resources ",")
    endif()
    string(APPEND _wiki_resources "{\"id\":\"${_page}\",\"title\":\"${_wiki_title_${_page}}\",\"topic\":\"framework-wiki\",\"kind\":\"guide\",\"path\":\"${_page}.html\",\"sha256\":\"${_hash}\"}")
endforeach()
file(WRITE "${_wiki_pack}/learning-pack.json"
    "{\"schema\":\"umicom.learning.pack\",\"version\":1,\"id\":\"framework.studio-wiki\",\"title\":\"Umicom Framework learning wiki\",\"product\":\"framework\",\"locale\":\"en\",\"revision\":1,\"compatibility\":{\"minimum\":[0,9,0],\"maximum\":[0,9,0]},\"resources\":[${_wiki_resources}]}\n")
umicom_bundle_learning_module(TARGET umicom-studio-ide NAME framework-wiki
    SOURCE_DIR "${_wiki_pack}" PACK_ID framework.studio-wiki PRODUCT framework)
