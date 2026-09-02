/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_views.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_VIEWS_H
#define UMICOM_STUDIO_WORKSPACE_VIEWS_H
#include "umicom/studio/professional_workspace.h"
/**
 * List the named studio workspace view kind values accepted by this public contract.
 */
typedef enum UmiStudioWorkspaceViewKind {
    UMI_STUDIO_WORKSPACE_VIEW_OVERVIEW = 1,
    UMI_STUDIO_WORKSPACE_VIEW_LAYOUTS,
    UMI_STUDIO_WORKSPACE_VIEW_NEW_WINDOW,
    UMI_STUDIO_WORKSPACE_VIEW_GROUPS,
    UMI_STUDIO_WORKSPACE_VIEW_THEMES,
    UMI_STUDIO_WORKSPACE_VIEW_PERSISTENCE
} UmiStudioWorkspaceViewKind;
/**
 * Represent the studio workspace view data shared with callers of this public contract.
 */
typedef struct UmiStudioWorkspaceView {
    UmiStudioWorkspaceViewKind kind;
    char view_id[UMI_UI_WORKSPACE_LAYOUT_ID_CAPACITY];
    char title[UMI_UI_WORKSPACE_LAYOUT_NAME_CAPACITY];
    char summary[384U];
    size_t item_count;
    bool available;
} UmiStudioWorkspaceView;
/**
 * Return the number of records represented by studio workspace view without changing their
 * state.
 */
size_t umi_studio_workspace_view_count(void);
/**
 * Provide the studio workspace view resolve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_view_resolve(const UmiStudioProfessionalWorkspace *workspace,const char *view_id,UmiStudioWorkspaceView *out_view);
#endif
