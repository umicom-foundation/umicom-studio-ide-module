/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/profiles/commands/run_current.inc
 * PURPOSE: Expose explicit launch-only execution through the shared command registry.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UMI_STUDIO_GTK_COMMAND_PROFILE(
    "run_current",
    "studio.build.run-current",
    "Run current executable",
    "media-playback-start-symbolic",
    "Run",
    "Launch the selected program without saving or rebuilding")
