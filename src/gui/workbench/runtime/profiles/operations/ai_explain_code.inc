/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/profiles/operations/ai_explain_code.inc
 *
 * PURPOSE:
 *   Describe the canonical registered Studio operation "Explain Code" for
 *   Search Everywhere and operational command dispatch. The command itself and
 *   all business behavior remain owned by the Framework command registry and
 *   the established Studio service facade.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
UMI_STUDIO_GTK_OPERATION_PROFILE(
    "ai_explain_code",
    "studio.ai.explain-code",
    "Explain Code",
    "AI",
    "Explain code in workspace context.",
    UMI_STUDIO_GTK_OPERATION_ARGUMENT_TEXT)
