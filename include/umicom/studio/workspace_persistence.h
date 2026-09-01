/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_persistence.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace persistence contract over reusable
 *   Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_PERSISTENCE_H
#define UMICOM_STUDIO_WORKSPACE_PERSISTENCE_H
#include "umicom/studio/professional_workspace.h"
#include "umicom/ui/workspace_customisation_persistence.h"

/* Export Studio's active layout through the Framework-owned portable codec. */
UmiStatus umi_studio_workspace_persistence_export_active(
    UmiStudioProfessionalWorkspace *workspace,
    uint64_t saved_at_ns,
    char *out_text,
    size_t capacity);
/* Import a layout using the safe default which rejects an existing identifier. */
UmiStatus umi_studio_workspace_persistence_import(
    UmiStudioProfessionalWorkspace *workspace,
    const char *text,
    bool activate);
/* Let advanced Studio surfaces choose explicit replacement and receive a
 * detailed report while all parsing and validation remain in Framework. */
UmiStatus umi_studio_workspace_persistence_import_with_report(
    UmiStudioProfessionalWorkspace *workspace,
    const char *text,
    bool activate,
    bool replace_existing,
    UmiUiWorkspaceImportReport *out_report);
#endif
