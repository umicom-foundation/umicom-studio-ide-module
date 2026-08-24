/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/documents.inc
 *
 * PURPOSE:
 *   Provide operational New/Open/Save/Save As/Close behavior over the existing
 *   Framework DocumentCoordinator.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_document_new(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(view_id, 0, sizeof(view_id));
    status = umi_document_coordinator_new(
        umi_studio_ui_documents(runtime->owner->ui),
        "untitled.c",
        view_id,
        sizeof(view_id));

    if (status == UMI_STATUS_OK) {
        status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id);
    }

    (void)runtime_refresh_services(runtime);
    return status;
}

static UmiStatus runtime_document_save(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentCoordinator *documents;

    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    documents = umi_studio_ui_documents(runtime->owner->ui);
    if (documents == NULL) return UMI_STATUS_UNAVAILABLE;

    if (umi_document_coordinator_active_snapshot(
            documents,
            &active) != UMI_STATUS_OK) {
        return UMI_STATUS_NOT_FOUND;
    }

    if (!active.has_path) {
        return runtime_choose_save_as(runtime);
    }

    return umi_document_coordinator_save_active(documents);
}

static UmiStatus runtime_document_close_force(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStatus status;

    status = umi_document_coordinator_close_active(
        umi_studio_ui_documents(runtime->owner->ui),
        1);
    (void)runtime_refresh_services(runtime);
    return status;
}

static UmiStatus runtime_document_close(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentCoordinator *documents;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    documents = umi_studio_ui_documents(runtime->owner->ui);
    if (documents == NULL) return UMI_STATUS_UNAVAILABLE;

    status = umi_document_coordinator_active_snapshot(
        documents,
        &active);
    if (status != UMI_STATUS_OK) return status;

    if (active.dirty) {
        return runtime_confirm(
            runtime,
            "Close Modified Document?",
            "The active document has unsaved changes. Closing it will "
            "discard those changes.",
            "Discard and Close",
            runtime_document_close_force);
    }

    status = umi_document_coordinator_close_active(documents, 0);
    (void)runtime_refresh_services(runtime);
    return status;
}
