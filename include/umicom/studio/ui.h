/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/ui.h
 *
 * PURPOSE:
 *   Own the toolkit-neutral Studio UI composition built from Umicom Framework
 *   workbench, application-shell, headless-adapter and command contracts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_UI_H
#define UMICOM_STUDIO_UI_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/umicom.h"
#include "umicom/platform/workspace_files.h"
#include "umicom/studio/application_surface.h"
#include "umicom/studio/application_surface_policy.h"
#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio ui data shared with callers of this public contract.
 */
typedef struct UmiStudioUi UmiStudioUi;
/**
 * Represent the studio view models data shared with callers of this public contract.
 */
typedef struct UmiStudioViewModels UmiStudioViewModels;

/**
 * Represent the studio ui snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioUiSnapshot {
    size_t perspectives;
    size_t panes;
    size_t documents;
    size_t actions;
    size_t menus;
    size_t toolbars;
    size_t status_items;
    size_t contributions;
    size_t notifications;
    size_t application_panels;
    size_t visible_application_panels;
    size_t ready_application_panels;
    size_t application_panels_needing_attention;
    size_t scheduled_application_panels;
    size_t streaming_application_panels;
    size_t guarded_application_panels;
    size_t context_enabled_application_panels;
    uint32_t checkpoint_interval_seconds;
    uint64_t workbench_revision;
    uint64_t render_revision;
    uint64_t application_surface_revision;
    char active_perspective[UMI_UI_ID_CAPACITY];
    char active_document[UMI_UI_ID_CAPACITY];
    char active_application_component[UMI_UI_ID_CAPACITY];
} UmiStudioUiSnapshot;

/**
 * Initialise studio ui from caller-provided values so later operations receive a known
 * state.
 */
UmiStatus umi_studio_ui_create(UmiStudioServices *services,
                               UmiCommandRegistry *commands,
                               UmiStudioUi **out_ui);
/**
 * Release or reset state held by studio ui so the same storage can be reused safely.
 */
void umi_studio_ui_destroy(UmiStudioUi *ui);
/**
 * Provide the studio ui publish operation used by this module and its client applications.
 */
UmiStatus umi_studio_ui_publish(UmiStudioUi *ui,
                                UmiServiceRegistry *registry);
/**
 * Provide the studio ui refresh operation used by this module and its client applications.
 */
UmiStatus umi_studio_ui_refresh(UmiStudioUi *ui);
/**
 * Provide the studio ui advance operation used by this module and its client applications.
 */
UmiStatus umi_studio_ui_advance(UmiStudioUi *ui, uint32_t elapsed_seconds);
/**
 * Provide the studio ui set background operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ui_set_background(UmiStudioUi *ui, int background);
/**
 * Provide the studio ui context changed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ui_context_changed(UmiStudioUi *ui,
                                        const char *component_id,
                                        const char *context_value);
/**
 * Provide the studio ui checkpoint due operation used by this module and its client
 * applications.
 */
int umi_studio_ui_checkpoint_due(const UmiStudioUi *ui,
                                 uint32_t elapsed_seconds,
                                 int changed);
/**
 * Provide the studio ui snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ui_snapshot(const UmiStudioUi *ui,
                                 UmiStudioUiSnapshot *out_snapshot);
/**
 * Provide the studio ui render headless operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_ui_render_headless(UmiStudioUi *ui,
                                        UmiUiHeadlessSnapshot *out_snapshot);

/**
 * Provide the studio ui services operation used by this module and its client
 * applications.
 */
UmiStudioServices *umi_studio_ui_services(UmiStudioUi *ui);
/**
 * Provide the studio ui shell operation used by this module and its client applications.
 */
UmiUiApplicationShell *umi_studio_ui_shell(UmiStudioUi *ui);
/**
 * Provide the studio ui workbench operation used by this module and its client
 * applications.
 */
UmiUiWorkbench *umi_studio_ui_workbench(UmiStudioUi *ui);
/**
 * Provide the studio ui headless operation used by this module and its client
 * applications.
 */
UmiUiHeadlessAdapter *umi_studio_ui_headless(UmiStudioUi *ui);
/**
 * Provide the studio ui view models operation used by this module and its client
 * applications.
 */
UmiStudioViewModels *umi_studio_ui_view_models(UmiStudioUi *ui);
/**
 * Provide the studio ui documents operation used by this module and its client
 * applications.
 */
UmiDocumentCoordinator *umi_studio_ui_documents(UmiStudioUi *ui);
/**
 * Provide the studio ui application surface operation used by this module and its client
 * applications.
 */
UmiStudioApplicationSurface *umi_studio_ui_application_surface(UmiStudioUi *ui);

/** Open a retained problem by stable ID through Framework's document service.
 * Existing unsaved drafts are selected, never overwritten. No build or save
 * is performed. Call from the UI owner thread. */
UmiStatus UmiStudioUiOpenProblem(UmiStudioUi *ui, const char *problemId);
/** Open the next/previous available source location in the shared diagnostics
 * model. A nonzero backwards flag selects the previous location; wraps once. */
UmiStatus UmiStudioUiNavigateProblem(UmiStudioUi *ui, int backwards);
/** Return whether the shared model contains any unresolved local source problem. */
int UmiStudioUiCanNavigateProblems(UmiStudioUi *ui);

/** Filesystem creation and editor adoption are separate outcomes. A created
 * file is kept if indexing or opening the tab fails, so retry cannot overwrite
 * it. Directory creation does not insert a directory into the file-only index. */
typedef struct UmiStudioProjectEntryResult {
    UmiWorkspaceEntryResult entry;
    UmiStatus index_status;
    UmiStatus document_status;
    char view_id[UMI_UI_ID_CAPACITY];
} UmiStudioProjectEntryResult;

/** Create an entry through Framework, then index/open a new file through the
 * existing services. Returns the creation status; inspect the two adoption
 * statuses separately. Call on the UI owner thread; a running build returns
 * BUSY. No execution trust is granted and no existing document is saved. */
UmiStatus UmiStudioUiCreateProjectEntry(UmiStudioUi *ui,
    uint64_t expectedRevision, const char *relativePath,
    UmiWorkspaceEntryKind kind, UmiStudioProjectEntryResult *outResult);

/** Refresh the open workspace's file list without scanning on the UI thread.
 * This read-only operation does not grant trust, save documents, execute tools,
 * or rediscover the workspace project graph. Call on the UI owner thread. */
UmiStatus UmiStudioUiRefreshProjectFiles(UmiStudioUi *ui);
/** Request cancellation of file-list refresh only; does not cancel a build. */
UmiStatus UmiStudioUiCancelProjectFileRefresh(UmiStudioUi *ui);
/** Copy Framework's refresh state for a status label; does not wait. */
UmiStatus UmiStudioUiProjectFileRefreshState(UmiStudioUi *ui,
    UmiFileIndexRefreshSnapshot *snapshot);

#ifdef __cplusplus
}
#endif

#endif
