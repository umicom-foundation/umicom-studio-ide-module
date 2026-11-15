/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/shortcut_reference.inc
 *
 * PURPOSE:
 *   Project the actual native keyboard catalogue through Framework into the existing Documentation preview.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/developer_workbench/shortcut_reference.h"
#include "umicom/ui/gtk4/text_preview.h"

/* Keyboard matching and this reference consume the same profile table. The
 * shared formatter owns validation; this adapter supplies native key labels and
 * current admission, then publishes through the existing documentation surface. */
static UmiStatus RuntimeShowShortcutReference(UmiStudioGtkRuntimeChrome *runtime, const char *argument)
{
    if (argument != NULL && argument[0] != '\0') return UMI_STATUS_INVALID_ARGUMENT;
    if (runtime == NULL) return UMI_STATUS_UNAVAILABLE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL, *guard = NULL;
    GtkWidget *location = NULL;
    UmiShortcutReferenceEntry *entries = NULL;
    char *text = NULL;
    UmiStatus status = UMI_STATUS_CANCELLED;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) goto done;
    if (g_object_get_data(window, "umicom-shortcut-reference-active") != NULL) {
        status = UMI_STATUS_BUSY; goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-shortcut-reference-active", GINT_TO_POINTER(1));
    status = runtime_workspace_open_surface(runtime, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status != UMI_STATUS_OK) goto done;
    const size_t count = sizeof RUNTIME_KEY_PROFILES / sizeof RUNTIME_KEY_PROFILES[0];
    if (count > UMI_SHORTCUT_REFERENCE_LIMIT) { status = UMI_STATUS_CAPACITY_EXCEEDED; goto done; }
    entries = g_try_new0(UmiShortcutReferenceEntry, count);
    if (count != 0U && entries == NULL) { status = UMI_STATUS_OUT_OF_MEMORY; goto done; }
    for (size_t i = 0U; i < count; ++i) {
        const UmiStudioGtkKeyProfile *profile = &RUNTIME_KEY_PROFILES[i];
        guint key = profile->key_upper == GDK_KEY_Tab ? GDK_KEY_Tab : profile->key_lower;
        char *label = gtk_accelerator_get_label(key, profile->modifiers);
        if (label == NULL) { status = UMI_STATUS_OUT_OF_MEMORY; goto done; }
        int truncated = g_strlcpy(entries[i].shortcut, label, sizeof entries[i].shortcut) >= sizeof entries[i].shortcut ||
            g_strlcpy(entries[i].command, profile->command_id, sizeof entries[i].command) >= sizeof entries[i].command ||
            g_strlcpy(entries[i].caption, profile->label, sizeof entries[i].caption) >= sizeof entries[i].caption;
        g_free(label);
        if (truncated) { status = UMI_STATUS_CAPACITY_EXCEEDED; goto done; }
        entries[i].available = runtime_command_enabled(runtime, profile->command_id) ? 1 : 0;
        /* Admission can consult an application provider. Re-resolve before the
         * next profile rather than treating the retained window as a runtime. */
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    }
    size_t bytes = 0U;
    status = UmiShortcutReferenceFormat(entries, count, NULL, 0U, &bytes);
    if (status != UMI_STATUS_OK) goto done;
    text = g_try_malloc(bytes + 1U);
    if (text == NULL) { status = UMI_STATUS_OUT_OF_MEMORY; goto done; }
    status = UmiShortcutReferenceFormat(entries, count, text, bytes + 1U, &bytes);
    if (status != UMI_STATUS_OK) goto done;
    if (runtime->documentation_location_entry == NULL ||
        runtime->documentation_summary_label == NULL || runtime->documentation_preview_view == NULL) {
        status = UMI_STATUS_UNAVAILABLE; goto done;
    }
    /* A generated reference has no file address. Clearing the address leaves
     * the browser's existing history intact; Reload can still revisit it. */
    location = g_object_ref(runtime->documentation_location_entry);
    gtk_editable_set_text(GTK_EDITABLE(location), "");
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL || gtk_editable_get_text(GTK_EDITABLE(location))[0] != '\0') {
        status = UMI_STATUS_CANCELLED; goto done;
    }
    status = UmiGtk4TextPreviewPublish(GTK_WINDOW(window),
        GTK_LABEL(runtime->documentation_summary_label),
        GTK_TEXT_VIEW(runtime->documentation_preview_view),
        "Keyboard shortcuts - current Studio bindings", text);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-shortcut-reference-active", NULL);
    g_free(text); g_free(entries); g_clear_object(&location);
    g_clear_object(&guard); g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}
