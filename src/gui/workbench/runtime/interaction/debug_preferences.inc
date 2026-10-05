/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_preferences.inc
 * PURPOSE: Read and save user-local debugger choices on workers while preserving unsaved drafts.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/debug_runtime/adapter_preferences.h"
#include "umicom/base/text.h"
typedef struct RuntimeDebugPreferenceRequest
{
    UmiStudioFileDialogRequest *owner;
    GtkWidget *kind, *entry;
    UmiDebugAdapterPreferences preferences;
    char path[UMI_PATH_CAPACITY];
    UmiStatus status;
    int save, inputsChanged, publishing;
} RuntimeDebugPreferenceRequest;
static void RuntimeDebugPreferenceChanged(GObject *object, gpointer data)
{
    RuntimeDebugPreferenceRequest *request = data;
    if (!request->publishing)
        request->inputsChanged = 1;
    else if (GTK_IS_EDITABLE(object) &&
             strcmp(gtk_editable_get_text(GTK_EDITABLE(object)), request->preferences.executable) != 0)
        request->inputsChanged = 1;
}
static void RuntimeDebugPreferenceKindChanged(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    RuntimeDebugPreferenceRequest *request = data;
    /* The worker may still be decoding preferences. Read its result only
     * during completion publication, after GTask has synchronized the threads. */
    if (!request->publishing)
    {
        request->inputsChanged = 1;
        return;
    }
    guint expected = strcmp(request->preferences.kind, "gdb") == 0 ? 0U : 1U;
    if (gtk_drop_down_get_selected(GTK_DROP_DOWN(object)) != expected)
        request->inputsChanged = 1;
}
static void RuntimeDebugPreferenceFree(gpointer data)
{
    RuntimeDebugPreferenceRequest *request = data;
    if (request == NULL)
        return;
    if (request->entry != NULL)
    {
        g_signal_handlers_disconnect_by_data(request->entry, request);
        if (g_object_get_data(G_OBJECT(request->entry), "umicom-debug-preference-pending") == request)
            g_object_set_data(G_OBJECT(request->entry), "umicom-debug-preference-pending", NULL);
    }
    if (request->kind != NULL)
        g_signal_handlers_disconnect_by_data(request->kind, request);
    g_clear_object(&request->entry);
    g_clear_object(&request->kind);
    runtime_file_dialog_request_free(request->owner);
    g_free(request);
}
/* Background work receives copied preferences and a resolved local path only.
 * Saving is explicit and may finish after the window closes. Loading publishes
 * a draft on the UI thread and never reconfigures a debugger by itself. */
static void RuntimeDebugPreferenceWorker(GTask *task, gpointer source, gpointer data, GCancellable *cancel)
{
    (void)source;
    (void)cancel;
    RuntimeDebugPreferenceRequest *request = data;
    request->status = UmiDebugAdapterPreferencesPath("Studio", NULL, request->path, sizeof(request->path));
    if (request->status == UMI_STATUS_OK && request->save)
    {
        GFile *file = g_file_new_for_path(request->path), *parent = g_file_get_parent(file);
        GError *error = NULL;
        if (parent == NULL || (!g_file_make_directory_with_parents(parent, NULL, &error) &&
                               !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_EXISTS)))
            request->status = UMI_STATUS_IO_ERROR;
        g_clear_error(&error);
        g_clear_object(&parent);
        g_object_unref(file);
    }
    if (request->status == UMI_STATUS_OK)
        request->status = request->save
                              ? UmiDebugAdapterPreferencesSave(request->path, &request->preferences)
                              : UmiDebugAdapterPreferencesLoad(request->path, &request->preferences);
    g_task_return_boolean(task, TRUE);
}
/* Re-resolve after each widget mutation: a notification can close the window
 * or replace its debugger panel, even while controls are retained by this task. */
static UmiStudioGtkRuntimeChrome *RuntimeDebugPreferenceOwner(RuntimeDebugPreferenceRequest *request,
                                                              GObject **window)
{
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, window);
    if (runtime == NULL || runtime->debug_adapter_kind != request->kind ||
        runtime->debug_adapter_executable != request->entry ||
        (gpointer)gtk_widget_get_root(request->entry) != (gpointer)*window)
        return NULL;
    return runtime;
}
static void RuntimeDebugPreferenceDone(GObject *source, GAsyncResult *result, gpointer context)
{
    (void)source;
    (void)context;
    RuntimeDebugPreferenceRequest *request = g_task_get_task_data(G_TASK(result));
    (void)g_task_propagate_boolean(G_TASK(result), NULL);
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeDebugPreferenceOwner(request, &window);
    if (runtime == NULL)
        goto done;
    if (!request->save && request->status == UMI_STATUS_OK && !request->inputsChanged)
    {
        request->publishing = 1;
        gtk_drop_down_set_selected(GTK_DROP_DOWN(request->kind),
                                   strcmp(request->preferences.kind, "gdb") == 0 ? 0U : 1U);
        g_clear_object(&window);
        runtime = RuntimeDebugPreferenceOwner(request, &window);
        if (runtime == NULL)
            goto done;
        if (!request->inputsChanged)
            gtk_editable_set_text(GTK_EDITABLE(request->entry), request->preferences.executable);
        g_clear_object(&window);
        runtime = RuntimeDebugPreferenceOwner(request, &window);
        if (runtime == NULL)
            goto done;
    }
    /* Clear the busy marker before the final notification. A new action can
     * then own its own request without the retiring task clearing its marker. */
    if (g_object_get_data(G_OBJECT(request->entry), "umicom-debug-preference-pending") == request)
        g_object_set_data(G_OBJECT(request->entry), "umicom-debug-preference-pending", NULL);
    char *message =
        request->status != UMI_STATUS_OK
            ? g_strdup_printf("Adapter preferences: %s. Location: %s. Existing draft values were retained.",
                              umi_status_text(request->status), request->path)
        : request->save ? g_strdup_printf("Adapter choice saved locally to %s. Use adapter applies the draft "
                                          "to this window; saving does not launch anything.",
                                          request->path)
        : request->inputsChanged ? g_strdup("Adapter fields changed during loading. The saved choice was not "
                                            "fully copied; review the draft or load it again.")
                                 : g_strdup("Saved adapter copied into the draft. Review it, then choose Use "
                                            "adapter. No debugger was started or changed.");
    runtime_show_notification(runtime, message);
    g_free(message);
done:
    g_clear_object(&window);
    /* GTask may lose its last reference on the worker thread. Detach the
     * request and release its GTK controls here, on the completion context. */
    g_task_set_task_data(G_TASK(result), NULL, NULL);
    RuntimeDebugPreferenceFree(request);
}
static void RuntimeDebugPreferenceClicked(GtkButton *button, gpointer context)
{
    UmiStudioGtkRuntimeChrome *runtime = context;
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->ui == NULL ||
        runtime->debug_adapter_kind == NULL || runtime->debug_adapter_executable == NULL)
        return;
    GtkWidget *entry = runtime->debug_adapter_executable;
    if (g_object_get_data(G_OBJECT(entry), "umicom-debug-preference-pending") != NULL)
    {
        runtime_show_notification(runtime, "Adapter preferences are already being read or saved.");
        return;
    }
    RuntimeDebugPreferenceRequest *request = g_new0(RuntimeDebugPreferenceRequest, 1);
    request->save = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "umicom-debug-preference-save"));
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    request->entry = g_object_ref(entry);
    request->kind = g_object_ref(runtime->debug_adapter_kind);
    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(request->kind));
    UmiStatus status = request->owner != NULL ? UMI_STATUS_OK : UMI_STATUS_OUT_OF_MEMORY;
    if (status == UMI_STATUS_OK && request->save)
    {
        if (selected > 1U)
            status = UMI_STATUS_INVALID_ARGUMENT;
        else
        {
            strcpy(request->preferences.kind, selected == 0U ? "gdb" : "lldb");
            status = umi_text_copy(request->preferences.executable, sizeof(request->preferences.executable),
                                   gtk_editable_get_text(GTK_EDITABLE(entry)));
        }
        if (status == UMI_STATUS_OK)
            status = UmiDebugAdapterPreferencesValidate(&request->preferences);
    }
    if (status != UMI_STATUS_OK)
    {
        RuntimeDebugPreferenceFree(request);
        runtime_show_notification(
            runtime,
            "Choose GDB or LLDB and a complete executable path, or leave the path blank, before saving.");
        return;
    }
    g_object_set_data(G_OBJECT(entry), "umicom-debug-preference-pending", request);
    g_signal_connect(entry, "changed", G_CALLBACK(RuntimeDebugPreferenceChanged), request);
    g_signal_connect(request->kind, "notify::selected", G_CALLBACK(RuntimeDebugPreferenceKindChanged),
                     request);
    GTask *task = g_task_new(NULL, NULL, RuntimeDebugPreferenceDone, NULL);
    g_task_set_task_data(task, request, NULL); /* Completion releases GTK-owned state. */
    g_task_run_in_thread(task, RuntimeDebugPreferenceWorker);
    g_object_unref(task);
    runtime_show_notification(runtime,
                              "Working with user-local adapter preferences. No debugger will be launched.");
}
