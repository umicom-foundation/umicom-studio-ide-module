/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_setup.inc
 * PURPOSE: Present explicit debug setup file actions and complete before/after review.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/debug/setup_document.h"
typedef struct RuntimeDebugSetupPanel
{
    GtkWidget *path, *title, *approval, *output;
    UmiDebugSetupReview *review;
    UmiDebugSetup *previous;
    uint64_t draftRevision;
    int busy;
    gulong pathChanged, titleChanged;
} RuntimeDebugSetupPanel;
typedef struct RuntimeDebugSetupRequest
{
    UmiStudioFileDialogRequest *owner;
    GtkWidget *panel;
    UmiDebugSetup *setup;
    UmiOutputFileSnapshot receipt;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status;
    uint64_t draftRevision;
    int save;
} RuntimeDebugSetupRequest;
static void RuntimeDebugSetupFree(gpointer data)
{
    RuntimeDebugSetupPanel *state = data;

    /* Retained entry widgets must lose their callbacks before the panel state
     * is released. Explicit references make disposal independent of child order. */
    if (state->pathChanged != 0U)
        g_signal_handler_disconnect(state->path, state->pathChanged);
    if (state->titleChanged != 0U)
        g_signal_handler_disconnect(state->title, state->titleChanged);
    g_clear_object(&state->path);
    g_clear_object(&state->title);
    g_clear_object(&state->approval);
    g_clear_object(&state->output);
    UmiDebugSetupReviewDestroy(state->review);
    UmiDebugSetupDestroy(state->previous);
    g_free(state);
}
/* Locate live parentage instead of storing a raw panel pointer on a button.
 * A retained button detached from its panel cannot reach retired state. */
static GtkWidget *RuntimeDebugSetupPanelFor(GtkWidget *widget)
{
    for (GtkWidget *p = widget; p != NULL; p = gtk_widget_get_parent(p))
        if (g_object_get_data(G_OBJECT(p), "umicom-debug-setup") != NULL)
            return p;
    return NULL;
}
static void RuntimeDebugSetupText(RuntimeDebugSetupPanel *state, const char *text)
{
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->output)), text, -1);
}
static void RuntimeDebugSetupChanged(GtkEditable *editable, gpointer data)
{

    (void)data;
    GtkWidget *panel = RuntimeDebugSetupPanelFor(GTK_WIDGET(editable));
    if (panel == NULL)
        return;
    g_object_ref(panel);
    RuntimeDebugSetupPanel *state = g_object_get_data(G_OBJECT(panel), "umicom-debug-setup");
    if (state->draftRevision != UINT64_MAX)
        ++state->draftRevision;
    UmiDebugSetupReviewDestroy(state->review);
    state->review = NULL;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(state->approval), FALSE);
    RuntimeDebugSetupText(state, "Fields changed. Load and review the selected file again before applying.");
    g_object_unref(panel);
}
static UmiStatus RuntimeDebugSetupShowReview(RuntimeDebugSetupPanel *state, UmiDebugWorkspace *workspace,
                                             const UmiDebugSetup *proposed)
{
    UmiDebugSetupReview *review = NULL;
    char *before = NULL, *after = NULL;
    size_t size = 0U;
    UmiStatus status = UmiDebugSetupReviewCreate(workspace, proposed, &review);
    if (status == UMI_STATUS_OK)
        status = UmiDebugSetupEncode(UmiDebugSetupReviewBefore(review), &before, &size);
    if (status == UMI_STATUS_OK)
        status = UmiDebugSetupEncode(UmiDebugSetupReviewAfter(review), &after, &size);
    if (status == UMI_STATUS_OK)
    {
        UmiDebugSetupReviewDestroy(state->review);
        state->review = review;
        review = NULL;
        char *text =
            g_strdup_printf("CURRENT SETTINGS\n%s\nPROPOSED SETTINGS\n%s\nApplying replaces both lists. "
                            "Stop debugging first. Expressions are retained without evaluation; inspect them "
                            "before a later debug session.",
                            before, after);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->approval), FALSE);
        RuntimeDebugSetupText(state, text);
        g_free(text);
    }
    UmiDebugSetupFreeBytes(before);
    UmiDebugSetupFreeBytes(after);
    UmiDebugSetupReviewDestroy(review);
    return status;
}
static void RuntimeDebugSetupWorker(GTask *task, gpointer source, gpointer data, GCancellable *cancel)
{
    (void)source;
    (void)cancel;
    RuntimeDebugSetupRequest *request = data;
    request->status = request->save ? UmiDebugSetupSaveNew(request->path, request->setup, &request->receipt)
                                    : UmiDebugSetupLoad(request->path, &request->setup);
    g_task_return_boolean(task, TRUE);
}
/* File work owns a copied setup and path only. Completion resolves the weak
 * window and live panel before touching UI or capturing current settings. A
 * changed path/title discards a load; a save always reports its captured path. */
static void RuntimeDebugSetupDone(GObject *source, GAsyncResult *result, gpointer data)
{
    (void)source;
    (void)data;
    RuntimeDebugSetupRequest *request = g_task_get_task_data(G_TASK(result));
    (void)g_task_propagate_boolean(G_TASK(result), NULL);
    RuntimeDebugSetupPanel *state = g_object_get_data(G_OBJECT(request->panel), "umicom-debug-setup");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
    state->busy = 0;
    g_object_set_data(G_OBJECT(request->panel), "umicom-debug-setup-pending", NULL);
    if (runtime != NULL && (gpointer)gtk_widget_get_root(request->panel) == (gpointer)window)
    {
        if (!request->save && request->status == UMI_STATUS_OK)
        {
            if (state->draftRevision != request->draftRevision)
                RuntimeDebugSetupText(
                    state, "Load finished, but the fields changed. No settings were applied; load again.");
            else
            {
                request->status =
                    RuntimeDebugSetupShowReview(state, runtime_debug_workspace(runtime), request->setup);
                if (request->status != UMI_STATUS_OK)
                    RuntimeDebugSetupText(state, umi_status_text(request->status));
            }
        }
        else
        {
            char *text = g_strdup_printf(
                "%s: %s\n%s%s", request->save ? "Save setup" : "Load setup", umi_status_text(request->status),
                request->path,
                request->save && request->status != UMI_STATUS_OK && request->receipt.path[0] != '\0'
                    ? "\nA partial new file may remain. Inspect it and choose a new name for another attempt."
                    : "");
            RuntimeDebugSetupText(state, text);
            g_free(text);
        }
    }
    g_clear_object(&window);
    g_task_set_task_data(G_TASK(result), NULL, NULL);
    UmiDebugSetupDestroy(request->setup);
    runtime_file_dialog_request_free(request->owner);
    g_object_unref(request->panel);
    g_free(request);
}
static void RuntimeDebugSetupFile(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    GtkWidget *panel = RuntimeDebugSetupPanelFor(GTK_WIDGET(button));
    if (panel == NULL)
        return;
    RuntimeDebugSetupPanel *state = g_object_get_data(G_OBJECT(panel), "umicom-debug-setup");
    if (state->busy)
    {
        RuntimeDebugSetupText(state, "A setup file operation is already in progress.");
        return;
    }
    RuntimeDebugSetupRequest *request = g_new0(RuntimeDebugSetupRequest, 1);
    request->save = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "umicom-debug-setup-save"));
    const char *path = gtk_editable_get_text(GTK_EDITABLE(state->path));
    UmiStatus status = UmiOutputFileValidatePath(path);
    if (status == UMI_STATUS_OK && strlen(path) >= sizeof request->path)
        status = UMI_STATUS_CAPACITY_EXCEEDED;
    if (status == UMI_STATUS_OK)
        memcpy(request->path, path, strlen(path) + 1U);
    if (status == UMI_STATUS_OK && request->save)
        status = UmiDebugSetupCapture(runtime_debug_workspace(runtime),
                                      gtk_editable_get_text(GTK_EDITABLE(state->title)), &request->setup);
    if (status == UMI_STATUS_OK)
    {
        request->owner = runtime_file_dialog_request_new(runtime, 0U);
        if (request->owner == NULL)
            status = UMI_STATUS_OUT_OF_MEMORY;
    }
    if (status != UMI_STATUS_OK)
    {
        UmiDebugSetupDestroy(request->setup);
        g_free(request);
        RuntimeDebugSetupText(state, umi_status_text(status));
        return;
    }
    request->panel = g_object_ref(panel);
    request->draftRevision = state->draftRevision;
    state->busy = 1;
    g_object_set_data(G_OBJECT(panel), "umicom-debug-setup-pending", request);
    UmiDebugSetupReviewDestroy(state->review);
    state->review = NULL;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(state->approval), FALSE);
    RuntimeDebugSetupText(state, request->save ? "Saving the captured settings to a new file..."
                                               : "Loading a setup for review...");
    GTask *task = g_task_new(NULL, NULL, RuntimeDebugSetupDone, NULL);
    g_task_set_task_data(task, request, NULL);
    g_task_run_in_thread(task, RuntimeDebugSetupWorker);
    g_object_unref(task);
}
static void RuntimeDebugSetupApply(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    GtkWidget *panel = RuntimeDebugSetupPanelFor(GTK_WIDGET(button));
    if (panel == NULL)
        return;
    /* Keep state alive across text/property notifications; refreshing Studio
     * happens last because it may rebuild this panel. */
    g_object_ref(panel);
    RuntimeDebugSetupPanel *state = g_object_get_data(G_OBJECT(panel), "umicom-debug-setup");
    UmiStudioFileDialogRequest *owner = runtime_file_dialog_request_new(runtime, 0U);
    UmiStatus status = UMI_STATUS_INVALID_STATE;
    UmiDebugSetup *previous = NULL;
    if (owner != NULL && !state->busy && state->review != NULL &&
        gtk_check_button_get_active(GTK_CHECK_BUTTON(state->approval)))
    {
        status = UmiDebugSetupCopy(UmiDebugSetupReviewBefore(state->review), &previous);
        if (status == UMI_STATUS_OK)
            status = UmiStudioDebuggerApplySetup(
                umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui)), state->review);
    }
    if (status == UMI_STATUS_OK)
    {
        UmiDebugSetupDestroy(state->previous);
        state->previous = previous;
        previous = NULL;
    }
    UmiDebugSetupDestroy(previous);
    UmiDebugSetupReviewDestroy(state->review);
    state->review = NULL;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(state->approval), FALSE);
    /* Refresh first and end with panel-owned state only. This avoids using a
     * runtime after callbacks from the final text notification close it. */

    GObject *window = NULL;
    runtime = owner != NULL ? runtime_file_dialog_owner(owner, &window) : NULL;
    if (status == UMI_STATUS_OK && runtime != NULL)
        (void)runtime_refresh_services(runtime);
    g_clear_object(&window);
    runtime_file_dialog_request_free(owner);
    RuntimeDebugSetupText(state, status == UMI_STATUS_OK
                                     ? "Setup applied to desired settings. Nothing was launched or "
                                       "evaluated. Review previous settings can prepare a reversal."
                                     : "Setup was not applied. Stop debugging, load a fresh review and "
                                       "confirm replacement; changed or stale settings are retained.");
    g_object_unref(panel);
}
static void RuntimeDebugSetupPrevious(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    GtkWidget *panel = RuntimeDebugSetupPanelFor(GTK_WIDGET(button));
    if (panel == NULL)
        return;
    g_object_ref(panel);
    RuntimeDebugSetupPanel *state = g_object_get_data(G_OBJECT(panel), "umicom-debug-setup");
    UmiStatus status =
        state->busy ? UMI_STATUS_BUSY
        : state->previous == NULL
            ? UMI_STATUS_NOT_FOUND
            : RuntimeDebugSetupShowReview(state, runtime_debug_workspace(runtime), state->previous);
    if (status != UMI_STATUS_OK)
        RuntimeDebugSetupText(state, umi_status_text(status));
    g_object_unref(panel);
}
static GtkWidget *RuntimeDebugSetupCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    RuntimeDebugSetupPanel *state = g_new0(RuntimeDebugSetupPanel, 1);
    g_object_set_data_full(G_OBJECT(panel), "umicom-debug-setup", state, RuntimeDebugSetupFree);
    gtk_box_append(GTK_BOX(panel), runtime_label("Saved breakpoint and watch setups", 0.0F));
    state->title = gtk_entry_new();
    state->path = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(state->title), "My debug setup");
    gtk_entry_set_placeholder_text(GTK_ENTRY(state->path),
                                   "Absolute path to a private .json file (new name to save)");
    const char *tags[] = {"title", "path"};
    GtkWidget *entries[] = {state->title, state->path};
    for (size_t i = 0U; i < 2U; ++i)
    {
        char tag[80];
        (void)snprintf(tag, sizeof tag, "studio.debug.setup.%s", tags[i]);
        (void)umi_gtk4_automation_tag_widget(entries[i], tag);
        gtk_box_append(GTK_BOX(panel), entries[i]);
        gtk_accessible_update_property(GTK_ACCESSIBLE(entries[i]), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                       i == 0U ? "Debug setup title" : "Debug setup file path", -1);
    }
    state->approval =
        gtk_check_button_new_with_label("Replace current breakpoints and watches with the reviewed settings");
    (void)umi_gtk4_automation_tag_widget(state->approval, "studio.debug.setup.approval");
    gtk_box_append(GTK_BOX(panel), state->approval);
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    const char *labels[] = {"Save new setup", "Load and review", "Apply reviewed setup",
                            "Review previous settings"};
    const char *actionsTags[] = {"save", "load", "apply", "previous"};
    for (size_t i = 0U; i < 4U; ++i)
    {
        GtkWidget *button = gtk_button_new_with_label(labels[i]);
        char tag[80];
        (void)snprintf(tag, sizeof tag, "studio.debug.setup.%s", actionsTags[i]);
        (void)umi_gtk4_automation_tag_widget(button, tag);
        if (i == 0U)
            g_object_set_data(G_OBJECT(button), "umicom-debug-setup-save", GINT_TO_POINTER(1));
        runtime_bind_editor_action(runtime, button,
                                   i < 2U    ? RuntimeDebugSetupFile
                                   : i == 2U ? RuntimeDebugSetupApply
                                             : RuntimeDebugSetupPrevious);
        gtk_box_append(GTK_BOX(actions), button);
    }
    gtk_box_append(GTK_BOX(panel), actions);
    state->output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(state->output), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(state->output), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(state->output), GTK_WRAP_WORD_CHAR);
    (void)umi_gtk4_automation_tag_widget(state->output, "studio.debug.setup.review");
    GtkWidget *scroll = runtime_scroll(state->output);
    gtk_widget_set_size_request(scroll, -1, 180);
    gtk_box_append(GTK_BOX(panel), scroll);
    RuntimeDebugSetupText(state, "Choose a private file path. Save creates a new file; Load shows every "
                                 "current and proposed setting. Nothing runs automatically.");

    g_object_ref(state->path);
    g_object_ref(state->title);
    g_object_ref(state->approval);
    g_object_ref(state->output);
    state->pathChanged = g_signal_connect(state->path, "changed", G_CALLBACK(RuntimeDebugSetupChanged), NULL);
    state->titleChanged =
        g_signal_connect(state->title, "changed", G_CALLBACK(RuntimeDebugSetupChanged), NULL);
    return panel;
}
