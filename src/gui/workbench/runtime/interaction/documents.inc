/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/documents.inc
 *
 * PURPOSE:
 *   Provide operational New/Open/Save/Save As/Close behavior over the existing
 *   Framework DocumentCoordinator.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_document_new(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(view_id, 0, sizeof(view_id));
    status = umi_document_coordinator_new(
        umi_studio_ui_documents(runtime->owner->ui),
        "untitled.c",
        view_id,
        sizeof(view_id));

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id);
    }

    (void)runtime_refresh_services(runtime);
    return status;
}

/*
 * Write runtime document in its stable representation and report capacity or input
 * failures to the caller.
 */
static UmiStatus runtime_document_save(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentCoordinator *documents;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    documents = umi_studio_ui_documents(runtime->owner->ui);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) return UMI_STATUS_UNAVAILABLE;

    /* Apply this operation only while the related capability or state is available. */
    if (umi_document_coordinator_active_snapshot(
            documents,
            &active) != UMI_STATUS_OK) {
        return UMI_STATUS_NOT_FOUND;
    }

    /* Apply this operation only while the related capability or state is available. */
    if (!active.has_path) {
        return runtime_choose_save_as(runtime);
    }

    return umi_document_coordinator_save_active(documents);
}

/* MIGRATION: Close decisions now use Framework's captured-state contract in
 * src/document/coordinator.c (UmiDocumentCoordinatorApplyClose) and the GTK
 * question in adapters/gtk4/document_commands_gtk4.c
 * (UmiGtk4AdapterRequestDocumentClose). Studio still displays completion through
 * runtime_document_edit_completed() in runtime_attach.inc.
 * The former ID-only forced-close path is preserved below for comparison. It
 * could approve discarding newer edits made after the question was displayed.
 * Do not enable it together with the shared call: that would attempt two closes.
 */
// /*
//  * Provide the runtime document close force operation used by this module and its client
//  * applications.
//  */
// static UmiStatus runtime_document_close_force(
//     UmiStudioGtkRuntimeChrome *runtime, UmiDocumentId documentId)
// {
//     UmiStatus status;
//
//     status = UmiDocumentCoordinatorClose(
//         umi_studio_ui_documents(runtime->owner->ui), documentId, 1);
//     (void)runtime_refresh_services(runtime);
//     return status;
// }
//
// /*
//  * Provide the runtime document close operation used by this module and its client
//  * applications.
//  */
// static UmiStatus runtime_document_close(
//     UmiStudioGtkRuntimeChrome *runtime)
// {
//     UmiDocumentWorkingCopySnapshot active;
//     UmiDocumentCoordinator *documents;
//     UmiStatus status;
//
//     /*
//      * Protect caller-owned memory by checking that required state is available before it is
//      * used.
//      */
//     if (runtime == NULL || runtime->owner == NULL) {
//         return UMI_STATUS_INVALID_ARGUMENT;
//     }
//
//     documents = umi_studio_ui_documents(runtime->owner->ui);
//     /*
//      * Protect caller-owned memory by checking that required state is available before it is
//      * used.
//      */
//     if (documents == NULL) return UMI_STATUS_UNAVAILABLE;
//
//     status = umi_document_coordinator_active_snapshot(
//         documents,
//         &active);
//     /* Preserve the original failure result so the caller can respond to the correct cause. */
//     if (status != UMI_STATUS_OK) return status;
//
//     /* Apply this operation only while the related capability or state is available. */
//     if (active.dirty) {
//         char message[512];
//         (void)snprintf(message, sizeof message,
//             "\"%s\" has unsaved changes. Discard and Close removes those edits. "
//             "Choose Cancel to return to the document and save it first.", active.display_name);
//         return runtime_confirm_document(runtime, "Close Modified Document?", message,
//             "Discard and Close", active.document_id, runtime_document_close_force);
//     }
//
//     status = umi_document_coordinator_close_active(documents, 0);
//     (void)runtime_refresh_services(runtime);
//     return status;
// }
//

static UmiStatus runtime_document_close(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return UmiGtk4AdapterRequestDocumentClose(runtime->owner->adapter, NULL);
}

/* Framework owns the save sequence. Studio presents its copied summary rather
 * than claiming that scheduling a filename dialog has already saved a file. */
static void runtime_save_all_completed(void *context,
    const UmiDocumentSaveProgress *progress)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || progress == NULL) return;
    char message[768];
    /* Presentation message construction now lives in Framework's
     * src/document/save_session.c: UmiDocumentSaveProgressFormat(). It checks
     * copied counters and bounded names before writing the summary. The former
     * Studio-only formatter is retained below for migration reference. */
    // if (progress->phase == UMI_DOCUMENT_SAVE_COMPLETE) {
    // (void)snprintf(message, sizeof(message),
    // "Save All complete: %zu saved, %zu already saved.",
    // progress->saved, progress->unchanged);
    // } else {
    // (void)snprintf(message, sizeof(message),
    // "Save All %s: %zu saved, %zu remaining. %s%s%s",
    // progress->phase == UMI_DOCUMENT_SAVE_CANCELLED ? "cancelled" : "stopped",
    // progress->saved, progress->remaining,
    // progress->display_name,
    // progress->display_name[0] != '\0' ? ": " : "",
    // umi_status_text(progress->last_status));
    // }
    if (UmiDocumentSaveProgressFormat(progress, message, sizeof(message)) != UMI_STATUS_OK)
        (void)snprintf(message, sizeof(message), "%s",
            "Save All progress could not be displayed. Keep the open drafts and check their saved state.");
    (void)runtime_refresh_services(runtime);
    runtime_show_notification(runtime, message);
}

static UmiStatus runtime_document_save_all(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Save All must not race a build that consumes the saved source files. */
    if (UmiStudioBuildBusy(umi_studio_services_build(
            umi_studio_ui_services(runtime->owner->ui))))
        return UMI_STATUS_BUSY;
    /* Former immediate return retained here for migration reference:
     * return UmiGtk4AdapterDocumentSaveAll(runtime->owner->adapter,
     *     runtime_save_all_completed, runtime);
     * Framework still owns the sequence. Studio additionally repaints command
     * availability immediately so Cancel Save All is usable between steps. */
    UmiStatus status = UmiGtk4AdapterDocumentSaveAll(runtime->owner->adapter,
        runtime_save_all_completed, runtime);
    if (status == UMI_STATUS_OK) {
        runtime_show_notification(runtime,
            "Save All started. Choose File > Cancel Save All to stop remaining saves.");
        (void)runtime_refresh_services(runtime);
    }
    return status;
}

/* Request cancellation through Framework; no draft or file is removed here. */
static UmiStatus RuntimeCancelSaveAll(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Framework's final summary reports cancellation and retained saved counts.
     * Do not reuse the host pointer after cancellation signals can detach it. */
    return UmiGtk4AdapterCancelDocumentSaveAll(runtime->owner->adapter);
}


/* Framework owns captured IDs, reviewed decisions and close progress. Studio
 * only displays its copied completion and asks its existing build service
 * whether source files are currently in use by a build. */
static void RuntimeCloseDocumentsCompleted(void *context,
    const UmiDocumentCloseProgress *progress)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || progress == NULL) return;
    char message[1024];
    if (UmiDocumentCloseProgressFormat(progress, message, sizeof(message)) != UMI_STATUS_OK)
        (void)snprintf(message, sizeof(message), "%s",
            "Check the remaining source tabs. Closing progress could not be displayed.");
    (void)runtime_refresh_services(runtime);
    runtime_show_notification(runtime, message);
}

static UmiStatus RuntimeCloseDocuments(UmiStudioGtkRuntimeChrome *runtime,
    UmiDocumentCloseScope scope)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioBuildBusy(umi_studio_services_build(
            umi_studio_ui_services(runtime->owner->ui)))) return UMI_STATUS_BUSY;
    UmiStatus status = UmiGtk4AdapterCloseDocuments(runtime->owner->adapter,
        scope, RuntimeCloseDocumentsCompleted, runtime);
    if (status == UMI_STATUS_OK) {
        runtime_show_notification(runtime,
            "Closing source documents. Review each unsaved draft; Cancel keeps remaining documents open.");
        (void)runtime_refresh_services(runtime);
    }
    return status;
}
