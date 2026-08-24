/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/explorer.inc
 *
 * PURPOSE:
 *   Turn the existing Framework UmiFileIndex into a clickable GTK Explorer.
 *   Indexing, path ownership and document loading remain Framework-owned.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_explorer_open_path(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *path)
{
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    if (runtime == NULL || path == NULL || path[0] == '\0') return;

    (void)memset(view_id, 0, sizeof(view_id));
    status = umi_document_coordinator_open(
        umi_studio_ui_documents(runtime->owner->ui),
        path,
        view_id,
        sizeof(view_id));

    if (status == UMI_STATUS_OK) {
        runtime_copy_selection(
            runtime->selected_file_path,
            sizeof(runtime->selected_file_path),
            path);
        status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id);
    }

    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
}

static void runtime_on_explorer_row_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *path = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-file-path");

    runtime_explorer_open_path(runtime, path);
}

static void runtime_explorer_refresh_list(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiFileIndex *index;
    UmiFileIndexStats stats;
    size_t position;
    size_t visible = 0U;
    char summary[192];

    if (runtime == NULL || runtime->explorer_list == NULL ||
        runtime->owner == NULL) {
        return;
    }

    runtime_clear_list_box(runtime->explorer_list);

    services = umi_studio_ui_services(runtime->owner->ui);
    index = services != NULL
        ? umi_studio_services_file_index(services)
        : NULL;

    if (index == NULL) {
        gtk_list_box_append(
            GTK_LIST_BOX(runtime->explorer_list),
            runtime_label("Workspace file index is unavailable.", 0.0F));
        return;
    }

    stats = umi_file_index_stats(index);

    for (position = 0U;
         position < stats.files &&
         visible < UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT;
         ++position) {
        UmiFileIndexEntry entry;
        GtkWidget *button;
        GtkWidget *row;
        GtkWidget *name;
        GtkWidget *path;

        if (umi_file_index_at(index, position, &entry) != UMI_STATUS_OK) {
            continue;
        }

        if (!runtime_text_contains_casefold(
                entry.relative_path,
                runtime->explorer_query)) {
            continue;
        }

        button = gtk_button_new();
        row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        name = runtime_label(entry.name, 0.0F);
        path = runtime_label(entry.relative_path, 0.0F);
        if (button == NULL || row == NULL ||
            name == NULL || path == NULL) {
            continue;
        }

        gtk_widget_add_css_class(button, "flat");
        gtk_widget_add_css_class(path, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(row), name);
        gtk_box_append(GTK_BOX(row), path);
        gtk_button_set_child(GTK_BUTTON(button), row);

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-file-path",
            g_strdup(entry.path),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_explorer_row_clicked),
            runtime);

        gtk_list_box_append(
            GTK_LIST_BOX(runtime->explorer_list),
            button);
        ++visible;
    }

    (void)snprintf(
        summary,
        sizeof(summary),
        "%zu of %zu indexed file%s%s",
        visible,
        stats.files,
        stats.files == 1U ? "" : "s",
        stats.files > UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT
            ? " (row limit applied)"
            : "");

    if (runtime->explorer_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->explorer_summary_label),
            summary);
    }
}

static void runtime_on_explorer_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *text;

    if (runtime == NULL) return;

    text = gtk_editable_get_text(GTK_EDITABLE(entry));
    runtime_copy_selection(
        runtime->explorer_query,
        sizeof(runtime->explorer_query),
        text);
    runtime_explorer_refresh_list(runtime);
}

static void runtime_on_explorer_refresh(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioServices *services;
    UmiFileIndex *index;
    UmiStatus status;
    (void)button;

    if (runtime == NULL || runtime->owner == NULL) return;

    services = umi_studio_ui_services(runtime->owner->ui);
    index = services != NULL
        ? umi_studio_services_file_index(services)
        : NULL;

    status = index != NULL
        ? umi_file_index_rebuild(index)
        : UMI_STATUS_UNAVAILABLE;

    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    runtime_explorer_refresh_list(runtime);
}
