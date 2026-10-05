/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/editor_controls.inc
 *
 * PURPOSE:
 *   Provide a compact professional editor command bar over the authoritative
 *   Framework DocumentCoordinator. Find, replace, line navigation, undo/redo,
 *   save and external-change checks remain document-model operations rather
 *   than GTK text-buffer business logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/text_comparison.h"

/* A toolbar may outlive its native owner while a test or accessibility client
 * retains a control. Resolve the existing weak owner before every action. */
typedef struct UmiStudioGtkEditorAction {
    UmiStudioFileDialogRequest *ownerRequest;
    void (*clicked)(GtkButton *button, gpointer runtime);
} UmiStudioGtkEditorAction;

static void runtime_editor_action_destroy(gpointer data, GClosure *closure)
{
    UmiStudioGtkEditorAction *action = data;
    (void)closure;
    if (action == NULL) return;
    runtime_file_dialog_request_free(action->ownerRequest);
    free(action);
}

static void runtime_editor_action_clicked(GtkButton *button, gpointer data)
{
    UmiStudioGtkEditorAction *action = data;
    GObject *ownerWindow = NULL;
    if (action == NULL) return;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(action->ownerRequest, &ownerWindow);
    /* A retained button from a removed panel is no longer a user action in
     * this window, even when the window itself is still alive. */
    if (runtime != NULL &&
        (gpointer)gtk_widget_get_root(GTK_WIDGET(button)) == (gpointer)ownerWindow)
        action->clicked(button, runtime);
    g_clear_object(&ownerWindow);
}

static void runtime_bind_editor_action(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *button, void (*clicked)(GtkButton *, gpointer))
{
    UmiStudioGtkEditorAction *action = calloc(1U, sizeof *action);
    if (action != NULL) action->ownerRequest = runtime_file_dialog_request_new(runtime, 0U);
    if (action == NULL || action->ownerRequest == NULL) {
        runtime_editor_action_destroy(action, NULL);
        gtk_widget_set_sensitive(button, FALSE);
        runtime_show_notification(runtime, "Could not attach an editor action. Close and reopen the editor panel.");
        return;
    }
    action->clicked = clicked;
    g_signal_connect_data(button, "clicked", G_CALLBACK(runtime_editor_action_clicked),
        action, runtime_editor_action_destroy, 0);
}

static UmiDocumentCoordinator *runtime_documents(
    UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL &&
           runtime->owner != NULL &&
           runtime->owner->ui != NULL
        ? umi_studio_ui_documents(runtime->owner->ui)
        : NULL;
}

/*
 * Provide the runtime editor update info operation used by this module and its client
 * applications.
 */
static void runtime_editor_update_info(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    char text[768];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->editor_info_label == NULL) return;

    (void)memset(&active, 0, sizeof(active));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime_documents(runtime) == NULL ||
        umi_document_coordinator_active_snapshot(
            runtime_documents(runtime),
            &active) != UMI_STATUS_OK) {
        gtk_label_set_text(
            GTK_LABEL(runtime->editor_info_label),
            "No active document");
        return;
    }

    (void)snprintf(
        text,
        sizeof(text),
        "%s%s — %s — %s — %s — %zu bytes — rev %llu%s",
        active.display_name,
        active.dirty ? " •" : "",
        active.language_id,
        umi_document_encoding_text(active.encoding),
        umi_document_line_ending_text(active.line_ending),
        active.text_length,
        (unsigned long long)active.revision,
        active.conflict == UMI_DOCUMENT_CONFLICT_EXTERNAL_CHANGE ? " — saved file changed" :
        active.conflict == UMI_DOCUMENT_CONFLICT_DELETED_EXTERNALLY ? " — saved file missing" : "");

    gtk_label_set_text(
        GTK_LABEL(runtime->editor_info_label),
        text);
}

/*
 * Provide the runtime on editor undo operation used by this module and its client
 * applications.
 */
/*
 * REFACTOR NOTE: Undo now uses the shared Framework editing-command path.
 *
 * The former Studio callback is retained below as comments so a developer can
 * compare its responsibilities with the active implementation. Undo has not
 * been removed. The document history already belonged to Framework; this
 * change replaces Studio's direct call with the shared GTK editing bridge.
 *
 * Active command and completion path (paths start at the Applications root):
 * 1. framework/adapters/gtk4/document_commands_gtk4.c:
 *    UmiGtk4AdapterDocumentCommand -> UmiGtk4EditorCommandForView resolves the
 *    intended document and delegates edit.undo to the existing coordinator.
 * 2. framework/src/document/coordinator.c:
 *    UmiDocumentCoordinatorUndo -> apply_history performs the history operation.
 * 3. framework/adapters/gtk4/document_commands_gtk4.c:
 *    EditFinished refreshes the Framework document views and invokes the
 *    registered application completion callback for a dispatched operation.
 * 4. applications/studio/src/gui/workbench/runtime/runtime_attach.inc:
 *    runtime_document_edit_completed reports applicable failures and calls
 *    runtime_refresh_services. runtime_attach registers that callback through
 *    UmiGtk4AdapterBindDocumentEditing.
 *
 * Studio still owns its notification and chrome refresh in the completion
 * callback. Those statements were consolidated there, not moved verbatim
 * into Framework. The reusable command dispatch is the Framework-owned part.
 *
 * The old body is documentation only, not an alternative or fallback path.
 * Do not run it alongside the active call: that could apply history twice and
 * duplicate result handling. Early command/document lookup failures return
 * before EditFinished; this note does not claim every failure invokes it.
 *
 * Former implementation from the baseline preceding Studio Edit Commands:
 */
// static void runtime_on_editor_undo(
//     GtkButton *button,
//     gpointer user_data)
// {
//     UmiStudioGtkRuntimeChrome *runtime =
//         (UmiStudioGtkRuntimeChrome *)user_data;
//     UmiStatus status;
//     (void)button;
//
//     status = umi_document_coordinator_undo(runtime_documents(runtime));
//     /* Preserve the original failure result so the caller can respond to the correct cause. */
//     if (status != UMI_STATUS_OK) {
//         runtime_show_notification(runtime, umi_status_text(status));
//     }
//     (void)runtime_refresh_services(runtime);
// }

static void runtime_on_editor_undo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /* The shared completion binding reports the result and refreshes once. */
    (void)UmiGtk4AdapterDocumentCommand(runtime->owner->adapter, "edit.undo");
}

/*
 * Provide the runtime on editor redo operation used by this module and its client
 * applications.
 */
/*
 * REFACTOR NOTE: Redo now uses the shared Framework editing-command path.
 *
 * The former Studio callback is retained below as comments so a developer can
 * compare its responsibilities with the active implementation. Redo has not
 * been removed. The document history already belonged to Framework; this
 * change replaces Studio's direct call with the shared GTK editing bridge.
 *
 * Active command and completion path (paths start at the Applications root):
 * 1. framework/adapters/gtk4/document_commands_gtk4.c:
 *    UmiGtk4AdapterDocumentCommand -> UmiGtk4EditorCommandForView resolves the
 *    intended document and delegates edit.redo to the existing coordinator.
 * 2. framework/src/document/coordinator.c:
 *    UmiDocumentCoordinatorRedo -> apply_history performs the history operation.
 * 3. framework/adapters/gtk4/document_commands_gtk4.c:
 *    EditFinished refreshes the Framework document views and invokes the
 *    registered application completion callback for a dispatched operation.
 * 4. applications/studio/src/gui/workbench/runtime/runtime_attach.inc:
 *    runtime_document_edit_completed reports applicable failures and calls
 *    runtime_refresh_services. runtime_attach registers that callback through
 *    UmiGtk4AdapterBindDocumentEditing.
 *
 * Studio still owns its notification and chrome refresh in the completion
 * callback. Those statements were consolidated there, not moved verbatim
 * into Framework. The reusable command dispatch is the Framework-owned part.
 *
 * The old body is documentation only, not an alternative or fallback path.
 * Do not run it alongside the active call: that could apply history twice and
 * duplicate result handling. Early command/document lookup failures return
 * before EditFinished; this note does not claim every failure invokes it.
 *
 * Former implementation from the baseline preceding Studio Edit Commands:
 */
// static void runtime_on_editor_redo(
//     GtkButton *button,
//     gpointer user_data)
// {
//     UmiStudioGtkRuntimeChrome *runtime =
//         (UmiStudioGtkRuntimeChrome *)user_data;
//     UmiStatus status;
//     (void)button;
//
//     status = umi_document_coordinator_redo(runtime_documents(runtime));
//     /* Preserve the original failure result so the caller can respond to the correct cause. */
//     if (status != UMI_STATUS_OK) {
//         runtime_show_notification(runtime, umi_status_text(status));
//     }
//     (void)runtime_refresh_services(runtime);
// }

static void runtime_on_editor_redo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /* The shared completion binding reports the result and refreshes once. */
    (void)UmiGtk4AdapterDocumentCommand(runtime->owner->adapter, "edit.redo");
}

/*
 * Write runtime on editor in its stable representation and report capacity or input
 * failures to the caller.
 */
static void runtime_on_editor_save(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    status = runtime_document_save(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}

/*
 * Find runtime on editor while leaving the underlying catalogue or model owned by this
 * module.
 */
/* Find now passes the toolbar case and word choices to the shared document search service.
 * The former implementation is retained for engineering review. */
#if 0
static void runtime_on_editor_find(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *needle;
    size_t offset = 0U;
    char message[192];
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->editor_find_entry == NULL) return;

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (needle == NULL || needle[0] == '\0') return;

    status = UmiDocumentCoordinatorFindNext(
        runtime_documents(runtime), needle, 0, &offset, NULL);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Match selected at byte %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}
#endif
static void runtime_on_editor_find(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *needle;
    size_t offset = 0U;
    char message[192];
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->editor_find_entry == NULL) return;

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (needle == NULL || needle[0] == '\0') return;

    UmiEditorSearchOptions searchOptions = RuntimeEditorSearchOptions(runtime);
    status = UmiDocumentCoordinatorFindWithOptions(
        runtime_documents(runtime), needle, &searchOptions, 0, &offset, NULL);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Match selected at byte %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}

/*
 * Provide the runtime on editor replace operation used by this module and its client
 * applications.
 */
/* Single replacement now shares the explicit search policy, including complete-document word-boundary checks.
 * The former implementation is retained for engineering review. */
#if 0
static void runtime_on_editor_replace(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *needle;
    const char *replacement;
    size_t offset = 0U;
    char message[192];
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->editor_find_entry == NULL ||
        runtime->editor_replace_entry == NULL) {
        return;
    }

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    replacement = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_replace_entry));

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (needle == NULL || needle[0] == '\0' ||
        replacement == NULL) {
        return;
    }

    status = UmiDocumentCoordinatorReplaceNext(
        runtime_documents(runtime),
        needle,
        replacement,
        &offset);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Replaced occurrence at offset %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
}
#endif
static void runtime_on_editor_replace(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *needle;
    const char *replacement;
    size_t offset = 0U;
    char message[192];
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->editor_find_entry == NULL ||
        runtime->editor_replace_entry == NULL) {
        return;
    }

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    replacement = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_replace_entry));

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (needle == NULL || needle[0] == '\0' ||
        replacement == NULL) {
        return;
    }

    UmiEditorSearchOptions searchOptions = RuntimeEditorSearchOptions(runtime);
    status = UmiDocumentCoordinatorReplaceNextWithOptions(
        runtime_documents(runtime),
        needle,
        replacement,
        &searchOptions,
        &offset);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Replaced occurrence at offset %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
}

/* Search backwards using the same Framework case and wrapping rules. */
/* Previous match uses the same captured toolbar search policy as forward navigation.
 * The former implementation is retained for engineering review. */
#if 0
static void runtime_on_editor_find_previous(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL || runtime->editor_find_entry == NULL) return;
    const char *needle = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_find_entry));
    if (needle == NULL || needle[0] == '\0') return;
    UmiStatus status = UmiDocumentCoordinatorFindNext(runtime_documents(runtime), needle, 1, NULL, NULL);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
    (void)runtime_refresh_services(runtime);
}
#endif
static void runtime_on_editor_find_previous(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL || runtime->editor_find_entry == NULL) return;
    const char *needle = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_find_entry));
    if (needle == NULL || needle[0] == '\0') return;
    UmiEditorSearchOptions searchOptions = RuntimeEditorSearchOptions(runtime);
    UmiStatus status = UmiDocumentCoordinatorFindWithOptions(runtime_documents(runtime), needle, &searchOptions, 1, NULL, NULL);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
    (void)runtime_refresh_services(runtime);
}

/* Replace All now presents an immutable Framework review before editing the draft, keeping stale-document checks and undo ownership in Framework. The previous implementation remains for engineering review. */
#if 0
/* Replace every occurrence as one undoable Framework edit. */
static void runtime_on_editor_replace_all(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    size_t count = 0U;
    char message[160];
    (void)button;
    if (runtime == NULL || runtime->editor_find_entry == NULL || runtime->editor_replace_entry == NULL) return;
    const char *needle = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_find_entry));
    const char *replacement = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_replace_entry));
    UmiStatus status = UmiDocumentCoordinatorReplaceAll(runtime_documents(runtime), needle, replacement, &count);
    if (status == UMI_STATUS_OK) {
        (void)snprintf(message, sizeof message, "Replaced %zu occurrence%s. Use Undo to reverse the change.",
            count, count == 1U ? "" : "s");
        runtime_show_notification(runtime, message);
    } else runtime_show_notification(runtime, umi_status_text(status));
    (void)runtime_refresh_services(runtime);
}
#endif
#include "replace_review.inc"

/*
 * Provide the runtime on editor goto operation used by this module and its client
 * applications.
 */
/* REFACTOR NOTE: The toolbar parser has moved from Studio to Framework:
 * src/document/commands.c::UmiDocumentLocationParse and
 * adapters/gtk4/document_commands_gtk4.c::UmiGtk4AdapterDocumentNavigate.
 * Studio retains presentation feedback in runtime_document_edit_completed.
 * The former code is retained below for reference; do not enable both paths.
 */
// static void runtime_on_editor_goto(
//     GtkButton *button,
//     gpointer user_data)
// {
//     UmiStudioGtkRuntimeChrome *runtime =
//         (UmiStudioGtkRuntimeChrome *)user_data;
//     const char *line_text;
//     char *end = NULL;
//     unsigned long line;
//     size_t offset = 0U;
//     char message[192];
//     UmiStatus status;
//     (void)button;
// 
//     /*
//      * Protect caller-owned memory by checking that required state is available before it is
//      * used.
//      */
//     if (runtime == NULL || runtime->editor_goto_entry == NULL) return;
// 
//     line_text = gtk_editable_get_text(
//         GTK_EDITABLE(runtime->editor_goto_entry));
//     /*
//      * Protect caller-owned memory by checking that required state is available before it is
//      * used.
//      */
//     if (line_text == NULL || line_text[0] == '\0') return;
// 
//     line = strtoul(line_text, &end, 10);
//     /* Apply this branch only when its contract condition is satisfied. */
//     if (end == line_text || *end != '\0' || line == 0UL) {
//         runtime_show_notification(runtime, "Enter a valid one-based line number.");
//         return;
//     }
// #if SIZE_MAX < ULONG_MAX
//     /* Apply this branch only when its contract condition is satisfied. */
//     if (line > (unsigned long)SIZE_MAX) {
//         runtime_show_notification(runtime, "Line number exceeds this platform's addressable range.");
//         return;
//     }
// #endif
// 
//     status = umi_document_coordinator_go_to_line(
//         runtime_documents(runtime),
//         (size_t)line,
//         &offset);
// 
//     /* Preserve the original failure result so the caller can respond to the correct cause. */
//     if (status == UMI_STATUS_OK) {
//         (void)snprintf(
//             message,
//             sizeof(message),
//             "Line %lu begins at document offset %zu.",
//             line,
//             offset);
//         runtime_show_notification(runtime, message);
//     } /* Use this fallback path when the earlier condition does not apply. */ else {
//         runtime_show_notification(runtime, umi_status_text(status));
//     }
// }

static void runtime_on_editor_goto(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    (void)button;
    if (runtime == NULL || runtime->owner == NULL || runtime->editor_goto_entry == NULL) return;
    const char *location = gtk_editable_get_text(GTK_EDITABLE(runtime->editor_goto_entry));
    /* Framework parses and navigates; Studio's existing completion callback
     * displays failures and refreshes the editor once. No source text is saved. */
    int reports = UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    UmiStatus status = UmiGtk4AdapterDocumentNavigate(runtime->owner->adapter, location);
    if (!reports && status != UMI_STATUS_OK)
        runtime_show_notification(runtime, umi_status_text(status));
}

/*
 * Provide the runtime on editor external check operation used by this module and its
 * client applications.
 */
static void runtime_on_editor_external_check(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiDocumentWorkingCopySnapshot active;
    int changed = 0;
    (void)button;
    UmiStatus status = umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active);
    if (status != UMI_STATUS_OK) { runtime_show_notification(runtime, umi_status_text(status)); return; }
    status = UmiDocumentCoordinatorCheckExternalChange(runtime_documents(runtime), active.document_id, &changed);
    if (status == UMI_STATUS_NOT_FOUND && changed)
        runtime_show_notification(runtime, "The saved file is missing. Your draft is kept; use Save As to preserve another copy.");
    else if (status == UMI_STATUS_OK)
        runtime_show_notification(runtime, changed
            ? "The saved file changed. Review it with Reload from disk, or keep your draft using Save As."
            : "No external file change detected.");
    else runtime_show_notification(runtime, umi_status_text(status));
    runtime_editor_update_info(runtime);
}

static void runtime_reload_plan_destroy(gpointer data)
{
    UmiDocumentReloadPlanDestroy(data);
}

static UmiStatus runtime_reload_accepted(UmiStudioGtkRuntimeChrome *runtime, void *data)
{
    UmiStatus status = UmiDocumentCoordinatorApplyReload(runtime_documents(runtime), data, 1);
    if (status == UMI_STATUS_OK) {
        runtime_show_notification(runtime, "Saved file reloaded. Undo restores the previous text; no file was written.");
        (void)runtime_refresh_services(runtime);
    }
    return status;
}

/* The Framework plan binds the question to this document and these bytes,
 * rather than whichever editor happens to be active when it is answered. */
static void runtime_on_editor_reload(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    UmiDocumentWorkingCopySnapshot active;
    UmiDocumentReloadSummary summary;
    UmiDocumentReloadPlan *plan = NULL;
    char message[1024];
    GtkWidget *review = NULL;
    (void)button;
    UmiStatus status = umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active);
    if (status == UMI_STATUS_OK)
        status = UmiDocumentCoordinatorPrepareReload(runtime_documents(runtime), active.document_id, &plan);
    if (status == UMI_STATUS_OK) status = UmiDocumentReloadPlanSummary(plan, &summary);
    if (status == UMI_STATUS_OK) {
        const char *previous = NULL, *incoming = NULL;
        size_t previousLength = 0U, incomingLength = 0U;
        status = UmiDocumentReloadPlanTexts(plan, &previous, &previousLength, &incoming, &incomingLength);
        if (status == UMI_STATUS_OK)
            status = UmiGtk4TextComparisonCreate(previous, previousLength, incoming, incomingLength,
                "Current editor draft (captured)", "Saved file (captured)", &review);
        if (review != NULL) g_object_ref_sink(review);
    }
    if (status == UMI_STATUS_OK) {
        (void)snprintf(message, sizeof message,
            "Reload %s from its saved file?\n\n%s\n\nCurrent text: %zu bytes. Saved text: %zu bytes. "
            "Saved encoding: %s; saved line endings: %s. "
            "The panes show decoded UTF-8 text with normalised line endings. "
            "The file on disk will not be changed. Undo can restore the previous text while the document remains open.",
            summary.display_name,
            summary.has_unsaved_changes
                ? "This will replace unsaved text in this document. Choose Cancel or use Save As to keep a separate copy first."
                : "The saved version will replace the text in this document.",
            summary.previous_bytes, summary.incoming_bytes,
            umi_document_encoding_text(summary.incoming_encoding),
            umi_document_line_ending_text(summary.incoming_line_ending));
        status = runtime_confirm_owned(runtime, "Compare with saved", message,
            "Reload", summary.document_id, runtime_reload_accepted, plan, runtime_reload_plan_destroy, review);
        if (status == UMI_STATUS_OK) plan = NULL; /* The dialog now owns it. */
    }
    g_clear_object(&review);
    UmiDocumentReloadPlanDestroy(plan);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
}

/*
 * Provide the runtime build editor command bar operation used by this module and its
 * client applications.
 */
/* The existing weak action binding rejects controls retained past shutdown. */
/* The weak editor binding resolves a live Studio host on every click. */
static void RuntimeReviewCompleteReplacementSet(GtkButton *button, gpointer data)
{
    (void)button;
    (void)RuntimeReviewReplacementSet(data);
}

static void runtime_on_editor_replace_open(GtkButton *button, gpointer user_data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    /* The route owns error feedback across possible host teardown. The former
     * direct feedback is retained for review; it could use a released runtime. */
#if 0
    UmiStatus status = RuntimeReviewOpenReplacements(runtime);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
#endif
    (void)RuntimeReviewOpenReplacements(runtime);
}

/* The existing weak toolbar binding resolves a current host for each click. */
static void runtime_on_editor_bookmark(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    int action = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "bookmark-action"));
    if (runtime != NULL && runtime->owner != NULL)
        (void)UmiGtk4AdapterBookmarkCommand(runtime->owner->adapter, action);
}

static void RuntimeSourceCompletion(GtkButton *button, gpointer data);
static void RuntimeSourceFormatting(GtkButton *button, gpointer data);
static void RuntimeSourceInformation(GtkButton *button, gpointer data);
static void RuntimeDocumentSymbols(GtkButton *button,gpointer data);
static void RuntimeParameterHelp(GtkButton *button,gpointer data);
static void RuntimeSymbolRename(GtkButton *button,gpointer data);
static void RuntimeCodeActions(GtkButton *button,gpointer data);
static void RuntimeSourceDiagnostics(GtkButton *button,gpointer data);
static void RuntimeSourceDefinition(GtkButton *button,gpointer data);
static void RuntimeSourceReferences(GtkButton *button,gpointer data);
static void RuntimeSourceTypeDefinition(GtkButton *button,gpointer data);
static void RuntimeSourceImplementation(GtkButton *button,gpointer data);
static void RuntimeSourceSelectionRanges(GtkButton *button,gpointer data);
static void RuntimeSourceFoldingRanges(GtkButton *button,gpointer data);
static void RuntimeSourceSnippet(GtkButton *button,gpointer data);
static void RuntimeFoldSelectedCode(GtkButton *button,gpointer data);
static void RuntimeSourceDelimiter(GtkButton *button,gpointer data);
static void RuntimeRevealFoldedCode(GtkButton *button,gpointer data);
static void RuntimeSourceCallers(GtkButton *button,gpointer data);
static void RuntimeSourceCallees(GtkButton *button,gpointer data);

/* Shared action groups reorganize the original, fully bound controls. */
#include "editor_action_groups.inc"
#include "editor_line_edits.inc"
#include "editor_document_format.inc"
#include "editor_recovery.inc"

static GtkWidget *runtime_build_editor_command_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *undo;
    GtkWidget *redo;
    GtkWidget *save;
    GtkWidget *find;
    GtkWidget *previous;
    GtkWidget *replace_all;
    GtkWidget *replace_open;
    GtkWidget *replace;
    GtkWidget *go_to;
    GtkWidget *external;
    GtkWidget *reload;
    GtkWidget *compare;
    GtkWidget *more;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    runtime->editor_find_entry = gtk_entry_new();
    runtime->editor_replace_entry = gtk_entry_new();
    runtime->editor_goto_entry = gtk_entry_new();
    runtime->editor_info_label = gtk_label_new("");

    undo = gtk_button_new_with_label("Undo");
    redo = gtk_button_new_with_label("Redo");
    save = gtk_button_new_with_label("Save");
    find = gtk_button_new_with_label("Find");
    previous = gtk_button_new_with_label("Previous");
    replace_all = gtk_button_new_with_label("Replace All");
    replace_open = gtk_button_new_with_label("Review Open Documents");
    gtk_widget_set_tooltip_text(replace_open,
        "Use Find and Replace with to review each open draft. Apply or Skip each document; Stop keeps earlier edits. Save remains separate.");
    (void)umi_gtk4_automation_tag_widget(replace_open, "studio.editor.replace-open-documents");
    runtime_bind_editor_action(runtime, replace_open, runtime_on_editor_replace_open);
    replace = gtk_button_new_with_label("Replace");
    go_to = gtk_button_new_with_label("Go");
    external = gtk_button_new_with_label("External?");
    reload = gtk_button_new_with_label("Reload from disk");
    compare = gtk_button_new_with_label("Compare with saved");
    more = runtime_context_actions_expander(runtime, "editor");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bar == NULL ||
        runtime->editor_find_entry == NULL ||
        runtime->editor_replace_entry == NULL ||
        runtime->editor_goto_entry == NULL ||
        runtime->editor_info_label == NULL ||
        undo == NULL || redo == NULL || save == NULL ||
        find == NULL || previous == NULL || replace_all == NULL || replace == NULL || go_to == NULL ||
        external == NULL || reload == NULL || compare == NULL) {
        return bar;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-editor-command-bar");
    gtk_widget_add_css_class(
        runtime->editor_info_label,
        "umicom-runtime-muted");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(runtime->editor_find_entry),
        "Find");
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(runtime->editor_replace_entry),
        "Replace with");
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(runtime->editor_goto_entry),
        /* Former placeholder: "Line". The same field now accepts a column. */
        "Line:column");
    gtk_widget_set_tooltip_text(runtime->editor_goto_entry,
        "Enter a line or line:column, for example 12:5. Columns count UTF-8 bytes.");

    /* Stable control names let native acceptance exercise the same fields. */
    (void)umi_gtk4_automation_tag_widget(runtime->editor_find_entry, "studio.editor.find-text");
    (void)umi_gtk4_automation_tag_widget(runtime->editor_replace_entry, "studio.editor.replace-text");
    gtk_widget_set_size_request(runtime->editor_find_entry, 150, -1);
    gtk_widget_set_size_request(runtime->editor_replace_entry, 150, -1);
    gtk_widget_set_size_request(runtime->editor_goto_entry, 70, -1);

    runtime_bind_editor_action(runtime, undo, runtime_on_editor_undo);
    runtime_bind_editor_action(runtime, redo, runtime_on_editor_redo);
    runtime_bind_editor_action(runtime, save, runtime_on_editor_save);
    runtime_bind_editor_action(runtime, find, runtime_on_editor_find);
    runtime_bind_editor_action(runtime, replace, runtime_on_editor_replace);
    runtime_bind_editor_action(runtime, go_to, runtime_on_editor_goto);
    runtime_bind_editor_action(runtime, external, runtime_on_editor_external_check);

    gtk_widget_set_tooltip_text(reload,
        "Review the saved version before replacing this document. Unsaved text requires confirmation; Undo restores the previous text.");
    (void)umi_gtk4_automation_tag_widget(reload, "studio.editor.reload");
    (void)umi_gtk4_automation_tag_widget(external, "studio.editor.check-external");
    runtime_bind_editor_action(runtime, reload, runtime_on_editor_reload);
    gtk_widget_set_tooltip_text(compare,
        "Compare the complete current draft with the saved file. Both panes are read-only; Cancel keeps your work unchanged.");
    (void)umi_gtk4_automation_tag_widget(compare, "studio.editor.compare-saved");
    runtime_bind_editor_action(runtime, compare, runtime_on_editor_reload);

    gtk_widget_set_tooltip_text(find, "Find the next occurrence. Search wraps at the end of the document.");
    gtk_widget_set_tooltip_text(previous, "Find the previous occurrence. Search wraps at the beginning.");
/* The toolbar now explains the review step before the shared replacement transaction. The previous implementation remains for engineering review. */
#if 0
    gtk_widget_set_tooltip_text(replace_all, "Replace all non-overlapping occurrences in this document. Undo restores the previous text.");
#endif
    gtk_widget_set_tooltip_text(replace_all, "Review all literal replacements in this document before applying. Cancel keeps the draft; Undo restores the reviewed text.");
    (void)umi_gtk4_automation_tag_widget(find, "studio.editor.find-next");
    (void)umi_gtk4_automation_tag_widget(previous, "studio.editor.find-previous");
    (void)umi_gtk4_automation_tag_widget(replace_all, "studio.editor.replace-all");
    (void)umi_gtk4_automation_tag_widget(replace, "studio.editor.replace-next");
    runtime_bind_editor_action(runtime, previous, runtime_on_editor_find_previous);
    runtime_bind_editor_action(runtime, replace_all, runtime_on_editor_replace_all);

    gtk_box_append(GTK_BOX(bar), undo);
    gtk_box_append(GTK_BOX(bar), redo);
    gtk_box_append(GTK_BOX(bar), save);
    /* Filename navigation shares Explorer's index and the document owner;
     * it does not launch a second scan or save the active source draft. */
    GtkWidget *quickOpen = gtk_button_new_with_label("Quick Open…");
    (void)umi_gtk4_automation_tag_widget(quickOpen, "studio.editor.quick-open");
    gtk_widget_set_tooltip_text(quickOpen, "Find a file in the current workspace index (Ctrl+P).");
    runtime_bind_editor_action(runtime, quickOpen, RuntimeQuickOpenClicked);
    gtk_box_append(GTK_BOX(bar), quickOpen);

    /* Keep the language action beside editing commands as well as in the
     * source-assistance panel; both routes use the same captured review. */
    GtkWidget *complete = gtk_button_new_with_label("Complete…");
    (void)umi_gtk4_automation_tag_widget(complete, "studio.editor.completion");
    runtime_bind_editor_action(runtime, complete, RuntimeSourceCompletion);
    gtk_widget_set_tooltip_text(complete, "Choose a local language server, request suggestions and review the result before changing this draft.");
    gtk_box_append(GTK_BOX(bar), complete);
    GtkWidget *format=gtk_button_new_with_label("Format…");
    (void)umi_gtk4_automation_tag_widget(format,"studio.editor.formatting");
    runtime_bind_editor_action(runtime,format,RuntimeSourceFormatting);
    /* Correct a misdecoded caption while preserving its original spelling
     * for review. The action and its automation identity are unchanged. */
#if 0
    GtkWidget *information=gtk_button_new_with_label("Source infoâ€¦");
#endif
    GtkWidget *information=gtk_button_new_with_label("Source info…");
    gtk_box_append(GTK_BOX(bar),information);
    (void)umi_gtk4_automation_tag_widget(information,"studio.editor.source-information");
    runtime_bind_editor_action(runtime,information,RuntimeSourceInformation);
    GtkWidget *symbols=gtk_button_new_with_label("Symbols…");
    (void)umi_gtk4_automation_tag_widget(symbols,"studio.editor.symbols");
    runtime_bind_editor_action(runtime,symbols,RuntimeDocumentSymbols);
    GtkWidget *parameters=gtk_button_new_with_label("Parameters…");
    (void)umi_gtk4_automation_tag_widget(parameters,"studio.editor.parameters");
    runtime_bind_editor_action(runtime,parameters,RuntimeParameterHelp);
    GtkWidget *rename=gtk_button_new_with_label("Rename…");
    (void)umi_gtk4_automation_tag_widget(rename,"studio.editor.rename");
    runtime_bind_editor_action(runtime,rename,RuntimeSymbolRename);
    GtkWidget *actions=gtk_button_new_with_label("Actions…");
    (void)umi_gtk4_automation_tag_widget(actions,"studio.editor.actions");
    runtime_bind_editor_action(runtime,actions,RuntimeCodeActions);
    GtkWidget *diagnostics=gtk_button_new_with_label("Problems…");
    (void)umi_gtk4_automation_tag_widget(diagnostics,"studio.editor.diagnostics");
    runtime_bind_editor_action(runtime,diagnostics,RuntimeSourceDiagnostics);
    gtk_box_append(GTK_BOX(bar),diagnostics);

    gtk_box_append(GTK_BOX(bar),actions);

    gtk_box_append(GTK_BOX(bar),rename);

    gtk_box_append(GTK_BOX(bar),parameters);

    gtk_box_append(GTK_BOX(bar),symbols);

    GtkWidget *definition=gtk_button_new_with_label("Definitions…");
    GtkWidget *references=gtk_button_new_with_label("References…");
    (void)umi_gtk4_automation_tag_widget(definition,"studio.editor.definition");
    (void)umi_gtk4_automation_tag_widget(references,"studio.editor.references");
    runtime_bind_editor_action(runtime,definition,RuntimeSourceDefinition);
    runtime_bind_editor_action(runtime,references,RuntimeSourceReferences);
    gtk_box_append(GTK_BOX(bar),definition);gtk_box_append(GTK_BOX(bar),references);
    /* Reuse Framework source navigation so every destination is reviewed and
     * checked against its current draft before the editor moves the caret. */
    GtkWidget *type_definition=gtk_button_new_with_label("Type definitions…");
    GtkWidget *implementation=gtk_button_new_with_label("Implementations…");
    (void)umi_gtk4_automation_tag_widget(type_definition,"studio.editor.type-definition");
    (void)umi_gtk4_automation_tag_widget(implementation,"studio.editor.implementation");
    runtime_bind_editor_action(runtime,type_definition,RuntimeSourceTypeDefinition);
    runtime_bind_editor_action(runtime,implementation,RuntimeSourceImplementation);
    gtk_widget_set_tooltip_text(type_definition,"Ask the local language server where the type of the symbol at the caret is defined.");
    gtk_widget_set_tooltip_text(implementation,"Ask the local language server for implementations of the symbol at the caret.");
    gtk_box_append(GTK_BOX(bar),type_definition);gtk_box_append(GTK_BOX(bar),implementation);
    /* The language service supplies syntax boundaries. Framework validates the
     * complete enclosing chain and moves the selection only after a choice. */
    GtkWidget *syntax_selection=gtk_button_new_with_label("Syntax selection…");
    (void)umi_gtk4_automation_tag_widget(syntax_selection,"studio.editor.syntax-selection");
    runtime_bind_editor_action(runtime,syntax_selection,RuntimeSourceSelectionRanges);
    gtk_widget_set_tooltip_text(syntax_selection,"Ask the selected language server for enclosing expressions and blocks at the captured caret.");
    gtk_box_append(GTK_BOX(bar),syntax_selection);
    /* A server supplies the structural regions; selecting one still passes
     * through Framework's captured-draft check. Manual Fold selection remains
     * available when a language server is absent or lacks folding support. */
    GtkWidget *syntax_folding=gtk_button_new_with_label("Syntax folding…");
    (void)umi_gtk4_automation_tag_widget(syntax_folding,"studio.editor.syntax-folding");
    runtime_bind_editor_action(runtime,syntax_folding,RuntimeSourceFoldingRanges);
    gtk_widget_set_tooltip_text(syntax_folding,"Find complete-line regions with the selected language server, select one, then use Fold selection.");
    gtk_box_append(GTK_BOX(bar),syntax_folding);

    /* Framework captures the draft and owns call-site navigation. Studio adds
     * only the two user intents and the per-language settings namespace. */
    GtkWidget *callers=gtk_button_new_with_label("Callers…"),*callees=gtk_button_new_with_label("Callees…");
    (void)umi_gtk4_automation_tag_widget(callers,"studio.editor.callers");
    (void)umi_gtk4_automation_tag_widget(callees,"studio.editor.callees");
    runtime_bind_editor_action(runtime,callers,RuntimeSourceCallers);
    runtime_bind_editor_action(runtime,callees,RuntimeSourceCallees);
    gtk_widget_set_tooltip_text(callers,"Inspect direct callers and their call sites through the selected language server.");
    gtk_widget_set_tooltip_text(callees,"Inspect direct callees and the expressions that call them.");
    gtk_box_append(GTK_BOX(bar),callers);gtk_box_append(GTK_BOX(bar),callees);
    /* Local snippet state and insertion review belong to Framework so every
     * editor host can use the same linked values and document history. */
    GtkWidget *snippet=gtk_button_new_with_label("Snippet…");
    (void)umi_gtk4_automation_tag_widget(snippet,"studio.editor.snippet");
    runtime_bind_editor_action(runtime,snippet,RuntimeSourceSnippet);
    gtk_widget_set_tooltip_text(snippet,"Compose linked template values, review the complete insertion, and apply it as one undoable edit.");
    gtk_box_append(GTK_BOX(bar),snippet);
    /* Keep folding as a view operation. Framework owns ranges and reveals
     * hidden source when edits or navigation invalidate the presentation. */
    GtkWidget *fold=gtk_button_new_with_label("Fold selection"),*reveal=gtk_button_new_with_label("Show all code");
    (void)umi_gtk4_automation_tag_widget(fold,"studio.editor.fold-selection");
    (void)umi_gtk4_automation_tag_widget(reveal,"studio.editor.reveal-folds");
    runtime_bind_editor_action(runtime,fold,RuntimeFoldSelectedCode);
    runtime_bind_editor_action(runtime,reveal,RuntimeRevealFoldedCode);
    gtk_widget_set_tooltip_text(fold,"Keep the first selected line visible and hide the following selected lines without changing source.");
    gtk_widget_set_tooltip_text(reveal,"Reveal every manual fold in this editor.");
    gtk_box_append(GTK_BOX(bar),fold);gtk_box_append(GTK_BOX(bar),reveal);
    /* Framework discovers complete balanced pairs and owns source navigation.
     * These actions work without starting a compiler or language server. */
    const char *delimiter_labels[]={"Matching bracket","Select inside brackets","Select bracket pair"};
    const char *delimiter_ids[]={"studio.editor.match-delimiter","studio.editor.select-delimiter-content","studio.editor.select-delimiter-pair"};
    for(size_t index=0U;index<3U;++index) {
        GtkWidget *delimiter=gtk_button_new_with_label(delimiter_labels[index]);
        (void)umi_gtk4_automation_tag_widget(delimiter,delimiter_ids[index]);
        g_object_set_data(G_OBJECT(delimiter),"document-delimiter-action",GINT_TO_POINTER((int)index));
        runtime_bind_editor_action(runtime,delimiter,RuntimeSourceDelimiter);
        gtk_widget_set_tooltip_text(delimiter,"Navigate balanced parentheses, square brackets and braces. C and JSON skip quoted content; other languages use literal matching.");
        gtk_box_append(GTK_BOX(bar),delimiter);
    }








    gtk_box_append(GTK_BOX(bar),format);

    gtk_box_append(GTK_BOX(bar), runtime->editor_find_entry);
    RuntimeAppendSearchOptions(runtime, bar);
    gtk_box_append(GTK_BOX(bar), find);
    gtk_box_append(GTK_BOX(bar), previous);
    gtk_box_append(GTK_BOX(bar), runtime->editor_replace_entry);
    gtk_box_append(GTK_BOX(bar), replace);
    gtk_box_append(GTK_BOX(bar), replace_all);
    gtk_box_append(GTK_BOX(bar), replace_open);
    GtkWidget *replace_set = gtk_button_new_with_label("Review Complete Set");
    gtk_widget_set_tooltip_text(replace_set,
        "Review all open draft replacements before applying the complete set together. No files are saved.");
    (void)umi_gtk4_automation_tag_widget(replace_set, "studio.editor.replace-complete-set");
    runtime_bind_editor_action(runtime, replace_set, RuntimeReviewCompleteReplacementSet);
    gtk_box_append(GTK_BOX(bar), replace_set);
    gtk_box_append(GTK_BOX(bar), runtime->editor_goto_entry);
    gtk_box_append(GTK_BOX(bar), go_to);
    gtk_box_append(GTK_BOX(bar), external);
    gtk_box_append(GTK_BOX(bar), compare);
    gtk_box_append(GTK_BOX(bar), reload);
    /* These are location metadata actions: drafts are neither saved nor edited. */
    const char *bookmarkLabels[] = {"Bookmark Line", "Next Bookmark", "Previous Bookmark"};
    const char *bookmarkTags[] = {"studio.editor.bookmark-toggle", "studio.editor.bookmark-next", "studio.editor.bookmark-previous"};
    const int bookmarkActions[] = {0, 1, -1};
    for (size_t index = 0U; index < 3U; ++index) {
        GtkWidget *bookmark = gtk_button_new_with_label(bookmarkLabels[index]);
        (void)umi_gtk4_automation_tag_widget(bookmark, bookmarkTags[index]);
        g_object_set_data(G_OBJECT(bookmark), "bookmark-action", GINT_TO_POINTER(bookmarkActions[index]));
        gtk_widget_set_tooltip_text(bookmark, "Session bookmarks remember fixed lines in open drafts. Use command search to Clear Bookmarks.");
        runtime_bind_editor_action(runtime, bookmark, runtime_on_editor_bookmark);
        gtk_box_append(GTK_BOX(bar), bookmark);
    }
    gtk_box_append(GTK_BOX(bar), runtime->editor_info_label);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (more != NULL) gtk_box_append(GTK_BOX(bar), more);

    runtime_editor_update_info(runtime);
    RuntimeAppendLineEdits(runtime,bar);
    RuntimeAppendDocumentFormats(runtime,bar);
    RuntimeGroupEditorActions(bar,save);
    /* Recovery is a document workflow and remains next to Save. */
    RuntimeAppendRecovery(runtime,bar,save);
    /* Keep every control reachable on a narrow workbench without increasing
     * the window's minimum width or removing existing actions. */
    GtkWidget *scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), bar);
    runtime->editor_command_bar = scroller;
    return scroller;
}

/* Studio selects its active document; Framework owns capture, connection
 * choice and request review. The action never reads saved files or clipboard
 * contents, and the native window does not retain the Studio runtime. */
/* The original entry assumed that native chat construction could not close
 * its workbench. The weak-identity entry below replaces that assumption while
 * retaining the same capture and messages. Keep the former code for review. */
#if 0
static void RuntimeSelectedCodeChat(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiDocumentWorkingCopySnapshot active;
    UmiStatus status = umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active);
    if (status == UMI_STATUS_OK) status = UmiProviderConnectionsGtkPresentSelection(
        runtime->owner->window, "studio", "desktop",
        umi_ui_workbench_documents(umi_studio_ui_workbench(runtime->owner->ui)), active.view_id);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
        status == UMI_STATUS_CAPACITY_EXCEEDED
            ? "Select at most 8192 bytes of code, then choose Chat about selected code again."
            : "Select text in the active editor before opening selected-code chat. The selection must contain complete UTF-8 text.");
}
#endif
/* Window construction can synchronously close the workbench. Retain its
 * weak identity before opening chat and resolve it again before notification;
 * the selected source still belongs to the existing Framework document owner. */
static void RuntimeSelectedCodeChat(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { runtime_show_notification(runtime, "Could not retain the source window for chat."); return; }
    UmiDocumentWorkingCopySnapshot active;
    UmiStatus status = umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active);
    if (status == UMI_STATUS_OK) status = UmiProviderConnectionsGtkPresentSelection(
        runtime->owner->window, "studio", "desktop",
        umi_ui_workbench_documents(umi_studio_ui_workbench(runtime->owner->ui)), active.view_id);
    GObject *owner_window = NULL;
    runtime = runtime_file_dialog_owner(request, &owner_window);
    if (runtime != NULL && status != UMI_STATUS_OK) runtime_show_notification(runtime,
        status == UMI_STATUS_CAPACITY_EXCEEDED
            ? "Select at most 8192 bytes of code, then choose Chat about selected code again."
            : "Select text in the active editor before opening selected-code chat. The selection must contain complete UTF-8 text.");
    g_clear_object(&owner_window);
    runtime_file_dialog_request_free(request);
}
/* Framework captures and commits the source draft. The weak request permits
 * native construction callbacks to close Studio without retaining its runtime. */
static void RuntimeSelectedCodeProposal(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { runtime_show_notification(runtime, "Could not retain the source window for review."); return; }
    UmiStatus status = UmiGtk4AdapterReviewSelectedCode(runtime->owner->adapter);
    GObject *owner_window = NULL;
    runtime = runtime_file_dialog_owner(request, &owner_window);
    if (runtime != NULL && status != UMI_STATUS_OK) runtime_show_notification(runtime,
        status == UMI_STATUS_BUSY ? "Finish the current document operation before opening a proposed edit."
            : "Select text in a writable source document before reviewing its proposed replacement.");
    g_clear_object(&owner_window);
    runtime_file_dialog_request_free(request);
}

/* Studio supplies the project folder and existing adapter. Framework owns
 * source capture, the worker connection, comparison and Undo transaction. */
static void RuntimeSourceCompletion(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewCompletionWithSettings(runtime->owner->adapter,folder,"Studio",NULL);
    /* The transient form remains available to other hosts. This former call is
     * retained for review while Studio adopts explicit local Save/Load controls. */
#if 0
    UmiStatus status=UmiGtk4AdapterReviewCompletion(runtime->owner->adapter,folder);
#endif
    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting completion.":
            "Open a writable source draft. Use a caret or a selection within one line and keep the draft within the completion transport limit.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}

static void RuntimeSourceFormatting(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewFormattingWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting formatting.":
            "Open a writable source draft within the language-server message limit. Formatting reviews the whole document.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}

/* Framework displays server documentation literally and owns the captured request.
 * Studio only supplies its current project folder and document binding. */
static void RuntimeSourceInformation(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewSourceInformationWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting source information.":
            "Open a source draft within the language-server message limit. Source information is shown at the captured caret.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* Framework owns the captured source and overload display; this action only
 * supplies Studio's current project and local settings namespace. */
static void RuntimeParameterHelp(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewParameterHelpWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting parameter help.":
            "Open a source draft within the language-server message limit. Parameter help is shown at the captured caret.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* Framework owns rename capture, proposal scope and approval; Studio supplies
 * only its project folder and local language-server settings namespace. */
static void RuntimeSymbolRename(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewSymbolRenameWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting symbol rename.":
            "Open a source draft within the language-server message limit. Rename requires a writable draft and a complete proposal for that document.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* Studio supplies context; Framework owns source capture, complete action
 * proposals and approval without application-specific edit logic. */
static void RuntimeCodeActions(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewCodeActionsWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting code actions.":
            "Open a source draft within the language-server message limit. Code actions require a writable draft and a complete edit-only proposal for that document.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* Framework owns diagnostic capture and exact source navigation; Studio
 * contributes the current workspace and its private server-settings location. */
static void RuntimeSourceDiagnostics(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewSourceDiagnosticsWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting source diagnostics.":
            "Open a source draft within the language-server message limit. Source diagnostics require an open source draft and a local language server that publishes diagnostics.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
/* The product delegates outline requests and opening to Framework so names
 * never become a separate editable document model. */
static void RuntimeDocumentSymbols(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewDocumentSymbolsWithSettings(runtime->owner->adapter,folder,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting document symbols.":
            "Open a source draft within the language-server message limit. Symbols describe the complete captured source draft.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}

/* Definitions and references share Framework inspection and document navigation.
 * The product selects an intent without owning another source model. */
static void RuntimeSourceNavigation(GtkButton *button, gpointer data,UmiLanguageNavigationKind kind)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewSourceNavigationWithSettings(runtime->owner->adapter,folder,kind,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting source navigation.":
            "Open a source draft within the language-server message limit. Review returned locations before opening a destination.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static void RuntimeSourceCalls(GtkButton *button, gpointer data,UmiLanguageCallDirection direction)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStudioServices *services=umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    const char *folder=NULL;
    if(services!=NULL && umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && workspace.graph.open)
        folder=workspace.graph.root;
    /* Framework owns per-language settings and background I/O. Studio supplies
     * its local namespace; loading a saved choice never starts a process. */
    UmiStatus status=UmiGtk4AdapterReviewCallHierarchyWithSettings(runtime->owner->adapter,folder,direction,"Studio",NULL);

    GObject *window=NULL;
    runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?
            "Finish the current document operation before requesting source navigation.":
            "Open a source draft within the language-server message limit. Review returned locations before opening a destination.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static void RuntimeSourceCallers(GtkButton *button,gpointer data) {RuntimeSourceCalls(button,data,UMI_LANGUAGE_CALL_INCOMING);}
static void RuntimeSourceCallees(GtkButton *button,gpointer data) {RuntimeSourceCalls(button,data,UMI_LANGUAGE_CALL_OUTGOING);}

static void RuntimeSourceDefinition(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_DEFINITION);
}
static void RuntimeSourceReferences(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_REFERENCES);
}
/* Studio selects an intent; Framework retains protocol and document ownership. */
static void RuntimeSourceTypeDefinition(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_TYPE_DEFINITION);
}
static void RuntimeSourceImplementation(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_IMPLEMENTATION);
}
static void RuntimeSourceSelectionRanges(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_SELECTION_RANGES);
}
static void RuntimeSourceFoldingRanges(GtkButton *button,gpointer data)
{
    RuntimeSourceNavigation(button,data,UMI_LANGUAGE_NAVIGATION_FOLDING_RANGES);
}





static void RuntimeSelectedCodeChatControls(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *box)
{
    GtkWidget *completion=gtk_button_new_with_label("Review language completion…");
    g_object_set_data_full(G_OBJECT(completion),"umicom-automation-id",g_strdup("studio.language.completion-review"),g_free);
    runtime_bind_editor_action(runtime,completion,RuntimeSourceCompletion);
    gtk_widget_set_tooltip_text(completion,"Request suggestions from a selected local language server, compare the full result and apply it with Undo. Opening the form sends nothing.");
    gtk_box_append(GTK_BOX(box),completion);

    GtkWidget *button = gtk_button_new_with_label("Chat about selected code…");
    g_object_set_data_full(G_OBJECT(button), "umicom-automation-id", g_strdup("studio.ai.selected-code"), g_free);
    runtime_bind_editor_action(runtime, button, RuntimeSelectedCodeChat);
    /* Framework owns the conversation in the new window. Make the explicit
     * follow-up route discoverable without keeping an editor reference. */
    gtk_widget_set_tooltip_text(button,
        "Choose a connection and review the selected code before sending. In the chat window, select an earlier reply excerpt to prepare a reviewed follow-up.");
    gtk_box_append(GTK_BOX(box), button);
    GtkWidget *proposal = gtk_button_new_with_label("Review selected-code replacement…");
    g_object_set_data_full(G_OBJECT(proposal), "umicom-automation-id", g_strdup("studio.ai.selected-code-proposal"), g_free);
    runtime_bind_editor_action(runtime, proposal, RuntimeSelectedCodeProposal);
    gtk_widget_set_tooltip_text(proposal,
        "Capture the source selection, type or paste proposed code, and inspect both complete drafts before applying. Undo restores the draft; Save remains separate.");
    gtk_box_append(GTK_BOX(box), proposal);
}

/* Named templates use the Studio local settings namespace; Framework continues to own all I/O, draft publication and insertion review.
 * The former implementation is retained for engineering review. */
#if 0
static void RuntimeSourceSnippet(GtkButton *button,gpointer data)
{
    (void)button;UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStatus status=UmiGtk4AdapterReviewSnippet(runtime->owner->adapter);
    GObject *window=NULL;runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?"Finish the current document operation before inserting a snippet.":"Open a writable source draft, then choose Snippet to compose and review an insertion.");
    g_clear_object(&window);runtime_file_dialog_request_free(request);
}
#endif
static void RuntimeSourceSnippet(GtkButton *button,gpointer data)
{
    (void)button;UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStatus status=UmiGtk4AdapterReviewSnippetWithSettings(runtime->owner->adapter,"Studio",NULL);
    GObject *window=NULL;runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && status!=UMI_STATUS_CANCELLED)
        runtime_show_notification(runtime,status==UMI_STATUS_BUSY?"Finish the current document operation before inserting a snippet.":"Open a writable source draft, then choose Snippet to compose and review an insertion.");
    g_clear_object(&window);runtime_file_dialog_request_free(request);
}

static void RuntimeSourceFolding(GtkButton *button,gpointer data,int reveal)
{
    (void)button;UmiStudioGtkRuntimeChrome *runtime=data;
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    UmiStatus status=reveal?UmiGtk4AdapterRevealFoldedCode(runtime->owner->adapter):UmiGtk4AdapterFoldSelectedCode(runtime->owner->adapter);
    GObject *window=NULL;runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL) runtime_show_notification(runtime,status==UMI_STATUS_OK?
        (reveal?"All manually folded code is visible.":"Selected code folded. The first line stays visible; Show all code reveals the rest."):
        "Select at least two source lines in the editor before folding. Source text has not been changed by folding.");
    g_clear_object(&window);runtime_file_dialog_request_free(request);
}
static void RuntimeFoldSelectedCode(GtkButton *button,gpointer data) {RuntimeSourceFolding(button,data,0);}
static void RuntimeRevealFoldedCode(GtkButton *button,gpointer data) {RuntimeSourceFolding(button,data,1);}

static void RuntimeSourceDelimiter(GtkButton *button,gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime=data;
    UmiDocumentDelimiterAction action=(UmiDocumentDelimiterAction)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button),"document-delimiter-action"));
    UmiStudioFileDialogRequest *request=runtime_file_dialog_request_new(runtime,0U);
    if(request==NULL) {runtime_show_notification(runtime,"Could not retain the editor window.");return;}
    int reports=UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    UmiStatus status=UmiGtk4AdapterNavigateDelimiter(runtime->owner->adapter,action);
    GObject *window=NULL;runtime=runtime_file_dialog_owner(request,&window);
    if(runtime!=NULL && status!=UMI_STATUS_OK && !reports) runtime_show_notification(runtime,
        "No complete bracket pair at this location. Matching uses a bracket at or just before the caret; selection uses an enclosing pair.");
    g_clear_object(&window);runtime_file_dialog_request_free(request);
}
