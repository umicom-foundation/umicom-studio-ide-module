/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/context_actions/build_compile.inc
 *
 * PURPOSE:
 *   Place the canonical "Build" operation into the build
 *   advanced-action section. The command behavior remains registered in the
 *   authoritative Framework command registry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_CONTEXT_ACTION(
    "build",
    "studio.build.compile",
    111U)
