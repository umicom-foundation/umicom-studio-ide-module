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

/*
 * Provide the runtime document close force operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_document_close_force(
    UmiStudioGtkRuntimeChrome *runtime, UmiDocumentId documentId)
{
    UmiStatus status;

    status = UmiDocumentCoordinatorClose(
        umi_studio_ui_documents(runtime->owner->ui), documentId, 1);
    (void)runtime_refresh_services(runtime);
    return status;
}

/*
 * Provide the runtime document close operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_document_close(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentCoordinator *documents;
    UmiStatus status;

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

    status = umi_document_coordinator_active_snapshot(
        documents,
        &active);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Apply this operation only while the related capability or state is available. */
    if (active.dirty) {
        char message[512];
        (void)snprintf(message, sizeof message,
            "\"%s\" has unsaved changes. Discard and Close removes those edits. "
            "Choose Cancel to return to the document and save it first.", active.display_name);
        return runtime_confirm_document(runtime, "Close Modified Document?", message,
            "Discard and Close", active.document_id, runtime_document_close_force);
    }

    status = umi_document_coordinator_close_active(documents, 0);
    (void)runtime_refresh_services(runtime);
    return status;
}

/* Framework owns the save sequence. Studio presents its copied summary rather
 * than claiming that scheduling a filename dialog has already saved a file. */
static void runtime_save_all_completed(void *context,
    const UmiDocumentSaveProgress *progress)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || progress == NULL) return;
    char message[768];
    if (progress->phase == UMI_DOCUMENT_SAVE_COMPLETE) {
        (void)snprintf(message, sizeof(message),
            "Save All complete: %zu saved, %zu already saved.",
            progress->saved, progress->unchanged);
    } else {
        (void)snprintf(message, sizeof(message),
            "Save All %s: %zu saved, %zu remaining. %s%s%s",
            progress->phase == UMI_DOCUMENT_SAVE_CANCELLED ? "cancelled" : "stopped",
            progress->saved, progress->remaining,
            progress->display_name,
            progress->display_name[0] != '\0' ? ": " : "",
            umi_status_text(progress->last_status));
    }
    (void)runtime_refresh_services(runtime);
    runtime_show_notification(runtime, message);
}

static UmiStatus runtime_document_save_all(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return UmiGtk4AdapterDocumentSaveAll(runtime->owner->adapter,
        runtime_save_all_completed, runtime);
}
