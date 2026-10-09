/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/commands/run.inc
 *
 * PURPOSE:
 *   GTK presentation metadata for the real Framework command developer.run.
 *   Enablement and execution are resolved from UmiCommandRegistry.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* The established Run action builds its target first. The explicit caption distinguishes it from Run current executable; the previous presentation is retained for review. The previous implementation is retained for engineering review. */
#if 0
UMI_STUDIO_GTK_COMMAND_PROFILE(
    "run",
    "run.start",
    "Run",
    "media-playback-start-symbolic",
    "Run",
    "Run the active target")
#endif
UMI_STUDIO_GTK_COMMAND_PROFILE(
    "run",
    "run.start",
    "Build and Run",
    "media-playback-start-symbolic",
    "Run",
    "Save, configure, build and run the active target")
