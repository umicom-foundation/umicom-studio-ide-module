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

/*
 * Provide the runtime on editor replace operation used by this module and its client
 * applications.
 */
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

/* Search backwards using the same Framework case and wrapping rules. */
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

/*
 * Provide the runtime on editor goto operation used by this module and its client
 * applications.
 */
static void runtime_on_editor_goto(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *line_text;
    char *end = NULL;
    unsigned long line;
    size_t offset = 0U;
    char message[192];
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->editor_goto_entry == NULL) return;

    line_text = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_goto_entry));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (line_text == NULL || line_text[0] == '\0') return;

    line = strtoul(line_text, &end, 10);
    /* Apply this branch only when its contract condition is satisfied. */
    if (end == line_text || *end != '\0' || line == 0UL) {
        runtime_show_notification(runtime, "Enter a valid one-based line number.");
        return;
    }
#if SIZE_MAX < ULONG_MAX
    /* Apply this branch only when its contract condition is satisfied. */
    if (line > (unsigned long)SIZE_MAX) {
        runtime_show_notification(runtime, "Line number exceeds this platform's addressable range.");
        return;
    }
#endif

    status = umi_document_coordinator_go_to_line(
        runtime_documents(runtime),
        (size_t)line,
        &offset);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Line %lu begins at document offset %zu.",
            line,
            offset);
        runtime_show_notification(runtime, message);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
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
        "Line");

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
    gtk_widget_set_tooltip_text(replace_all, "Replace all non-overlapping occurrences in this document. Undo restores the previous text.");
    (void)umi_gtk4_automation_tag_widget(find, "studio.editor.find-next");
    (void)umi_gtk4_automation_tag_widget(previous, "studio.editor.find-previous");
    (void)umi_gtk4_automation_tag_widget(replace_all, "studio.editor.replace-all");
    runtime_bind_editor_action(runtime, previous, runtime_on_editor_find_previous);
    runtime_bind_editor_action(runtime, replace_all, runtime_on_editor_replace_all);

    gtk_box_append(GTK_BOX(bar), undo);
    gtk_box_append(GTK_BOX(bar), redo);
    gtk_box_append(GTK_BOX(bar), save);
    gtk_box_append(GTK_BOX(bar), runtime->editor_find_entry);
    gtk_box_append(GTK_BOX(bar), find);
    gtk_box_append(GTK_BOX(bar), previous);
    gtk_box_append(GTK_BOX(bar), runtime->editor_replace_entry);
    gtk_box_append(GTK_BOX(bar), replace);
    gtk_box_append(GTK_BOX(bar), replace_all);
    gtk_box_append(GTK_BOX(bar), runtime->editor_goto_entry);
    gtk_box_append(GTK_BOX(bar), go_to);
    gtk_box_append(GTK_BOX(bar), external);
    gtk_box_append(GTK_BOX(bar), compare);
    gtk_box_append(GTK_BOX(bar), reload);
    gtk_box_append(GTK_BOX(bar), runtime->editor_info_label);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (more != NULL) gtk_box_append(GTK_BOX(bar), more);

    runtime_editor_update_info(runtime);
    /* Keep every control reachable on a narrow workbench without increasing
     * the window's minimum width or removing existing actions. */
    GtkWidget *scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroller), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), bar);
    runtime->editor_command_bar = scroller;
    return scroller;
}
