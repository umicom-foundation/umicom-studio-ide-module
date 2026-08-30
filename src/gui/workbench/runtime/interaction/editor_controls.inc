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

static UmiDocumentCoordinator *runtime_documents(
    UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL &&
           runtime->owner != NULL &&
           runtime->owner->ui != NULL
        ? umi_studio_ui_documents(runtime->owner->ui)
        : NULL;
}

static void runtime_editor_update_info(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDocumentWorkingCopySnapshot active;
    char text[768];

    if (runtime == NULL || runtime->editor_info_label == NULL) return;

    (void)memset(&active, 0, sizeof(active));
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
        "%s%s — %s — %s — %s — %zu bytes — rev %llu",
        active.display_name,
        active.dirty ? " •" : "",
        active.language_id,
        umi_document_encoding_text(active.encoding),
        umi_document_line_ending_text(active.line_ending),
        active.text_length,
        (unsigned long long)active.revision);

    gtk_label_set_text(
        GTK_LABEL(runtime->editor_info_label),
        text);
}

static void runtime_on_editor_undo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    status = umi_document_coordinator_undo(runtime_documents(runtime));
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}

static void runtime_on_editor_redo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    status = umi_document_coordinator_redo(runtime_documents(runtime));
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}

static void runtime_on_editor_save(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    status = runtime_document_save(runtime);
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }
    (void)runtime_refresh_services(runtime);
}

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

    if (runtime == NULL || runtime->editor_find_entry == NULL) return;

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    if (needle == NULL || needle[0] == '\0') return;

    status = umi_document_coordinator_find(
        runtime_documents(runtime),
        needle,
        &offset);

    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Found at document offset %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
}

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

    if (runtime == NULL ||
        runtime->editor_find_entry == NULL ||
        runtime->editor_replace_entry == NULL) {
        return;
    }

    needle = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_find_entry));
    replacement = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_replace_entry));

    if (needle == NULL || needle[0] == '\0' ||
        replacement == NULL) {
        return;
    }

    status = umi_document_coordinator_replace(
        runtime_documents(runtime),
        needle,
        replacement,
        &offset);

    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Replaced occurrence at offset %zu.",
            offset);
        runtime_show_notification(runtime, message);
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
}

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

    if (runtime == NULL || runtime->editor_goto_entry == NULL) return;

    line_text = gtk_editable_get_text(
        GTK_EDITABLE(runtime->editor_goto_entry));
    if (line_text == NULL || line_text[0] == '\0') return;

    line = strtoul(line_text, &end, 10);
    if (end == line_text || *end != '\0' || line == 0UL) {
        runtime_show_notification(runtime, "Enter a valid one-based line number.");
        return;
    }
#if SIZE_MAX < ULONG_MAX
    if (line > (unsigned long)SIZE_MAX) {
        runtime_show_notification(runtime, "Line number exceeds this platform's addressable range.");
        return;
    }
#endif

    status = umi_document_coordinator_go_to_line(
        runtime_documents(runtime),
        (size_t)line,
        &offset);

    if (status == UMI_STATUS_OK) {
        (void)snprintf(
            message,
            sizeof(message),
            "Line %lu begins at document offset %zu.",
            line,
            offset);
        runtime_show_notification(runtime, message);
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
}

static void runtime_on_editor_external_check(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    int changed = 0;
    UmiStatus status;
    (void)button;

    status = umi_document_coordinator_check_external_change(
        runtime_documents(runtime),
        &changed);

    if (status == UMI_STATUS_OK) {
        runtime_show_notification(
            runtime,
            changed
                ? "The active file changed outside Studio."
                : "No external file change detected.");
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
}

static GtkWidget *runtime_build_editor_command_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar;
    GtkWidget *undo;
    GtkWidget *redo;
    GtkWidget *save;
    GtkWidget *find;
    GtkWidget *replace;
    GtkWidget *go_to;
    GtkWidget *external;
    GtkWidget *more;

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
    replace = gtk_button_new_with_label("Replace");
    go_to = gtk_button_new_with_label("Go");
    external = gtk_button_new_with_label("External?");
    more = runtime_context_actions_expander(runtime, "editor");

    if (bar == NULL ||
        runtime->editor_find_entry == NULL ||
        runtime->editor_replace_entry == NULL ||
        runtime->editor_goto_entry == NULL ||
        runtime->editor_info_label == NULL ||
        undo == NULL || redo == NULL || save == NULL ||
        find == NULL || replace == NULL || go_to == NULL ||
        external == NULL) {
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

    g_signal_connect(
        undo, "clicked",
        G_CALLBACK(runtime_on_editor_undo), runtime);
    g_signal_connect(
        redo, "clicked",
        G_CALLBACK(runtime_on_editor_redo), runtime);
    g_signal_connect(
        save, "clicked",
        G_CALLBACK(runtime_on_editor_save), runtime);
    g_signal_connect(
        find, "clicked",
        G_CALLBACK(runtime_on_editor_find), runtime);
    g_signal_connect(
        replace, "clicked",
        G_CALLBACK(runtime_on_editor_replace), runtime);
    g_signal_connect(
        go_to, "clicked",
        G_CALLBACK(runtime_on_editor_goto), runtime);
    g_signal_connect(
        external, "clicked",
        G_CALLBACK(runtime_on_editor_external_check), runtime);

    gtk_box_append(GTK_BOX(bar), undo);
    gtk_box_append(GTK_BOX(bar), redo);
    gtk_box_append(GTK_BOX(bar), save);
    gtk_box_append(GTK_BOX(bar), runtime->editor_find_entry);
    gtk_box_append(GTK_BOX(bar), find);
    gtk_box_append(GTK_BOX(bar), runtime->editor_replace_entry);
    gtk_box_append(GTK_BOX(bar), replace);
    gtk_box_append(GTK_BOX(bar), runtime->editor_goto_entry);
    gtk_box_append(GTK_BOX(bar), go_to);
    gtk_box_append(GTK_BOX(bar), external);
    gtk_box_append(GTK_BOX(bar), runtime->editor_info_label);
    if (more != NULL) gtk_box_append(GTK_BOX(bar), more);

    runtime_editor_update_info(runtime);
    runtime->editor_command_bar = bar;
    return bar;
}
