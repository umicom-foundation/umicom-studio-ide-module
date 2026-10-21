/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_search_keyboard.inc
 * PURPOSE: Connect native search-field keyboard input to Framework document search.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Field callbacks hold only the existing weak host token. They also verify the
 * current field identity: a retained entry from a removed surface must not act
 * on a replacement editor or a newly attached workbench. */
static void RuntimeSearchRequestDestroyed(gpointer data, GClosure *closure)
{
    (void)closure;
    runtime_file_dialog_request_free(data);
}

static UmiStudioGtkRuntimeChrome *RuntimeSearchEntryOwner(GtkWidget *entry,
    UmiStudioFileDialogRequest *request, GObject **window)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, window);
    return runtime != NULL && entry != NULL && entry == runtime->editor_find_entry &&
        gtk_widget_get_root(entry) != NULL ? runtime : NULL;
}

static void RuntimeSearchEntryActivated(GtkEntry *entry, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeSearchEntryOwner(GTK_WIDGET(entry), data, &window);
    if (runtime != NULL) (void)RuntimeFindActiveDocument(runtime, 0);
    g_clear_object(&window);
}

static gboolean RuntimeSearchEntryKey(GtkEventControllerKey *controller,
    guint keyval, guint keycode, GdkModifierType state, gpointer data)
{
    (void)keycode;
    GdkModifierType modifiers = state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK |
        GDK_ALT_MASK | GDK_SUPER_MASK | GDK_META_MASK);
    int previous = (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) &&
        modifiers == GDK_SHIFT_MASK;
    int escape = keyval == GDK_KEY_Escape && modifiers == 0;
    if (!previous && !escape) return FALSE;
    GtkWidget *entry = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeSearchEntryOwner(entry, data, &window);
    gboolean handled = runtime != NULL;
    if (runtime != NULL) {
        if (previous) (void)RuntimeFindActiveDocument(runtime, 1);
        else (void)RuntimePresentActiveSource(runtime);
    }
    g_clear_object(&window);
    return handled;
}

/* Enter in the location field uses exactly the same parser and completion as
 * the Go button. A field retained from a prior surface has no authority. */
static void RuntimeLocationEntryActivated(GtkEntry *entry, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && GTK_WIDGET(entry) == runtime->editor_goto_entry &&
        gtk_widget_get_root(GTK_WIDGET(entry)) != NULL)
        runtime_on_editor_goto(NULL, runtime);
    g_clear_object(&window);
}

/* Enter replaces one selected or next match through the same Framework owner
 * as the Replace button. It never applies an unreviewed Replace All operation. */
static void RuntimeReplaceEntryActivated(GtkEntry *entry, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL && GTK_WIDGET(entry) == runtime->editor_replace_entry &&
        gtk_widget_get_root(GTK_WIDGET(entry)) != NULL)
        (void)RuntimeReplaceActiveMatch(runtime);
    g_clear_object(&window);
}

static void RuntimeBindSearchKeyboard(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *entry = runtime->editor_find_entry;
    UmiStudioFileDialogRequest *activate = runtime_file_dialog_request_new(runtime, 0U);
    UmiStudioFileDialogRequest *keys = runtime_file_dialog_request_new(runtime, 0U);
    UmiStudioFileDialogRequest *location = runtime_file_dialog_request_new(runtime, 0U);
    UmiStudioFileDialogRequest *replace = runtime_file_dialog_request_new(runtime, 0U);
    if (activate == NULL || keys == NULL || location == NULL || replace == NULL) {
        runtime_file_dialog_request_free(activate);
        runtime_file_dialog_request_free(keys);
        runtime_file_dialog_request_free(location);
        runtime_file_dialog_request_free(replace);
        return; /* The existing Find and Previous buttons remain available. */
    }
    g_signal_connect_data(entry, "activate", G_CALLBACK(RuntimeSearchEntryActivated),
        activate, RuntimeSearchRequestDestroyed, 0);
    g_signal_connect_data(runtime->editor_goto_entry, "activate", G_CALLBACK(RuntimeLocationEntryActivated),
        location, RuntimeSearchRequestDestroyed, 0);
    g_signal_connect_data(runtime->editor_replace_entry, "activate", G_CALLBACK(RuntimeReplaceEntryActivated),
        replace, RuntimeSearchRequestDestroyed, 0);
    gtk_widget_set_tooltip_text(runtime->editor_replace_entry,
        "Enter replaces one match using the current Find options. Undo restores the edit. Ctrl+H: focus Replace.");
    GtkEventController *controller = gtk_event_controller_key_new();
    gtk_event_controller_set_name(controller, "studio.editor.find-keys");
    gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
    g_signal_connect_data(controller, "key-pressed", G_CALLBACK(RuntimeSearchEntryKey),
        keys, RuntimeSearchRequestDestroyed, 0);
    gtk_widget_add_controller(entry, controller);
    gtk_widget_set_tooltip_text(entry,
        "Find in the active document. Enter: next match. Shift+Enter: previous match. Escape: return to the editor. Ctrl+F: focus Find.");
}

/* Find and Replace now share field focusing with the same weak host and native field identity checks. Retain the Find-only implementation for review. */
#if 0
static UmiStatus RuntimeFocusEditorFind(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiDocumentWorkingCopySnapshot active;
    if (umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active) != UMI_STATUS_OK)
        return UMI_STATUS_NOT_FOUND;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, active.document_id);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = RuntimePresentActiveSource(runtime);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK && runtime->editor_find_entry != NULL) {
        /* A focus observer can replace the surface. Keep the native entry alive
         * while focusing, then select text only if it is still the current one. */
        GtkWidget *entry = g_object_ref(runtime->editor_find_entry);
        gboolean focused = gtk_widget_grab_focus(entry);
        g_clear_object(&window);
        runtime = runtime_file_dialog_owner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        else if (!focused || runtime->editor_find_entry != entry) status = UMI_STATUS_UNAVAILABLE;
        else gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        g_object_unref(entry);
    } else if (status == UMI_STATUS_OK) status = UMI_STATUS_UNAVAILABLE;
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

#endif
static UmiStatus RuntimeFocusEditorSearchField(UmiStudioGtkRuntimeChrome *runtime, int replace)
{
    if (runtime == NULL || runtime->owner == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiDocumentWorkingCopySnapshot active;
    if (umi_document_coordinator_active_snapshot(runtime_documents(runtime), &active) != UMI_STATUS_OK)
        return UMI_STATUS_NOT_FOUND;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, active.document_id);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = RuntimePresentActiveSource(runtime);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    GtkWidget *field = runtime == NULL ? NULL :
        (replace ? runtime->editor_replace_entry : runtime->editor_find_entry);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status == UMI_STATUS_OK && field != NULL) {
        /* A focus observer can replace the surface. Keep the native entry alive
         * while focusing, then select text only if it is still the current one. */
        GtkWidget *entry = g_object_ref(field);
        gboolean focused = gtk_widget_grab_focus(entry);
        g_clear_object(&window);
        runtime = runtime_file_dialog_owner(request, &window);
        if (runtime == NULL) status = UMI_STATUS_CANCELLED;
        else if (!focused || (replace ? runtime->editor_replace_entry : runtime->editor_find_entry) != entry) status = UMI_STATUS_UNAVAILABLE;
        else gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        g_object_unref(entry);
    } else if (status == UMI_STATUS_OK) status = UMI_STATUS_UNAVAILABLE;
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

static UmiStatus RuntimeFocusEditorFind(UmiStudioGtkRuntimeChrome *runtime)
{
    return RuntimeFocusEditorSearchField(runtime, 0);
}

static UmiStatus RuntimeFocusEditorReplace(UmiStudioGtkRuntimeChrome *runtime)
{
    return RuntimeFocusEditorSearchField(runtime, 1);
}
