/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/explorer.inc
 *
 * PURPOSE:
 *   Turn the existing Framework UmiFileIndex into a clickable GTK Explorer.
 *   Indexing, path ownership and document loading remain Framework-owned.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/explorer.h"

static void runtime_explorer_open_path(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *path)
{
    char view_id[UMI_UI_ID_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || path == NULL || path[0] == '\0') return;

    (void)memset(view_id, 0, sizeof(view_id));
    status = umi_document_coordinator_open(
        umi_studio_ui_documents(runtime->owner->ui),
        path,
        view_id,
        sizeof(view_id));

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        runtime_copy_selection(
            runtime->selected_file_path,
            sizeof(runtime->selected_file_path),
            path);
        status = umi_ui_workbench_activate_document(
            umi_studio_ui_workbench(runtime->owner->ui),
            view_id);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    (void)runtime_refresh_services(runtime);
}

/* A retained old row cannot reopen a previous workspace after refresh or
 * teardown. Reuse the window's existing runtime-generation token. */
typedef struct RuntimeExplorerRow {
    UmiStudioFileDialogRequest *owner;
    GObject *generation;
    GWeakRef list;
    char *root;
    char *path;
} RuntimeExplorerRow;

static void RuntimeExplorerRowFree(gpointer data)
{
    RuntimeExplorerRow *row = data;
    if (row == NULL) return;
    runtime_file_dialog_request_free(row->owner);
    g_clear_object(&row->generation);
    g_weak_ref_clear(&row->list);
    g_free(row->root);
    g_free(row->path);
    g_free(row);
}

/*
 * Provide the runtime on explorer row clicked operation used by this module and its client
 * applications.
 */
static void runtime_on_explorer_row_clicked(GtkButton *button, gpointer user_data)
{
    RuntimeExplorerRow *row = user_data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(row->owner, &window);
    GObject *list = g_weak_ref_get(&row->list);
    (void)button;
    if (runtime != NULL && list != NULL && runtime->explorer_list == GTK_WIDGET(list) &&
        g_object_get_data(list, "umicom-explorer-rows") == row->generation) {
        UmiWorkspaceGraphSnapshot workspace;
        if (umi_workspace_graph_snapshot(umi_studio_services_workspace(
                umi_studio_ui_services(runtime->owner->ui)), &workspace) == UMI_STATUS_OK &&
            workspace.open && umi_path_equal(workspace.root, row->root))
            runtime_explorer_open_path(runtime, row->path);
    }
    g_clear_object(&list);
    g_clear_object(&window);
}

/* Existing main-thread refresh ticks read copied state; the worker never
 * touches widgets. Updating this label does not rebuild unchanged file rows. */
static void RuntimeExplorerRefreshState(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiFileIndexRefreshSnapshot refresh;
    UmiWorkspaceGraphSnapshot workspace;
    UmiStudioUi *ui = runtime->owner->ui;
    char message[256];
    int open = umi_workspace_graph_snapshot(umi_studio_services_workspace(
        umi_studio_ui_services(ui)), &workspace) == UMI_STATUS_OK && workspace.open;
    UmiStatus status = UmiStudioUiProjectFileRefreshState(ui, &refresh);
    if (status != UMI_STATUS_OK) {
        (void)snprintf(message, sizeof(message), "Refresh status: %s", umi_status_text(status));
    } else if (!open) {
        (void)snprintf(message, sizeof(message), "Open a workspace to refresh its files.");
    } else if (refresh.requestId != 0U && !umi_path_equal(refresh.sourceRoot, workspace.root)) {
        (void)snprintf(message, sizeof(message), "%s", refresh.active ?
            "Finishing the previous workspace refresh; its results cannot replace this file list." :
            "Refresh checks the project for file changes.");
    } else if (refresh.active) {
        (void)snprintf(message, sizeof(message), "%s: %zu files scanned; the previous list remains available.",
            refresh.cancellationRequested ? "Cancelling refresh" : "Refreshing",
            refresh.filesScanned);
    } else if (refresh.requestId == 0U) {
        (void)snprintf(message, sizeof(message), "Refresh checks the project for file changes.");
    } else if (refresh.status == UMI_STATUS_OK) {
        (void)snprintf(message, sizeof(message), "Refresh complete: %zu files scanned.", refresh.filesScanned);
    } else {
        (void)snprintf(message, sizeof(message), "Refresh: %s. No partial scan was published.",
            umi_status_text(refresh.status));
    }
    if (runtime->explorer_refresh_status != NULL)
        gtk_label_set_text(GTK_LABEL(runtime->explorer_refresh_status), message);
    if (runtime->explorer_refresh_button != NULL)
        gtk_widget_set_sensitive(runtime->explorer_refresh_button,
            status == UMI_STATUS_OK && open && !refresh.active);
    if (runtime->explorer_cancel_refresh != NULL)
        gtk_widget_set_sensitive(runtime->explorer_cancel_refresh,
            status == UMI_STATUS_OK && refresh.active && !refresh.cancellationRequested);
}

/* Create one non-interactive folder heading for the current indexed page. */
static GtkWidget *RuntimeExplorerGroupRow(const char *labelText)
{
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *icon = gtk_image_new_from_icon_name("folder-symbolic");
    GtkWidget *label = runtime_label(labelText != NULL ? labelText : "", 0.0F);

    if (row == NULL || icon == NULL || label == NULL) return row;
    gtk_widget_add_css_class(row, "umicom-runtime-section-title");
    gtk_widget_set_margin_top(row, 5);
    gtk_widget_set_margin_bottom(row, 2);
    gtk_box_append(GTK_BOX(row), icon);
    gtk_box_append(GTK_BOX(row), label);
    return row;
}

/*
 * Provide the runtime explorer refresh list operation used by this module and its client
 * applications.
 */
static void runtime_explorer_refresh_list(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiFileIndex *index;
    UmiFileIndexStats stats;
    UmiFileIndexPage page;
    UmiFileIndexEntry *entries;
    UmiStatus status;
    char summary[256];
    char workspaceName[UMI_FILE_INDEX_NAME_CAPACITY];
    if (runtime == NULL || runtime->explorer_list == NULL || runtime->owner == NULL) return;
    services = umi_studio_ui_services(runtime->owner->ui);
    index = services != NULL ? umi_studio_services_file_index(services) : NULL;
    if (index == NULL) return;
    RuntimeExplorerRefreshState(runtime);
    stats = umi_file_index_stats(index);
    /* Status ticks must not replace unchanged buttons and steal keyboard focus. */
    if (runtime->explorer_rendered && runtime->explorer_revision == stats.revision &&
        runtime->explorer_rendered_offset == runtime->explorer_offset &&
        strcmp(runtime->explorer_rendered_query, runtime->explorer_query) == 0) return;
    entries = g_try_new0(UmiFileIndexEntry, UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT);
    if (entries == NULL) return;
    status = UmiFileIndexReadPage(index, runtime->explorer_query, 0,
        runtime->explorer_offset, stats.revision, entries,
        UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT, &page);
    /* Removing the final item on the last page returns to the last usable page. */
    if (status == UMI_STATUS_OK && page.count == 0U && runtime->explorer_offset != 0U) {
        runtime->explorer_offset = page.matched == 0U ? 0U :
            ((page.matched - 1U) / UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT) *
                UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT;
        status = UmiFileIndexReadPage(index, runtime->explorer_query, 0,
            runtime->explorer_offset, page.stats.revision, entries,
            UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT, &page);
    }
    if (status != UMI_STATUS_OK) { g_free(entries); return; }
    GObject *generation = g_object_new(G_TYPE_OBJECT, NULL);
    GPtrArray *rows = g_ptr_array_new_with_free_func(g_object_unref);

    /*
     * The previous flat two-line list is retained for review. Framework now
     * supplies portable folder context, so Studio can group and indent project
     * files without duplicating path parsing or changing File Index ownership.
     */
#if 0
    for (size_t i = 0U; i < page.count; ++i) {
        RuntimeExplorerRow *row = g_new0(RuntimeExplorerRow, 1);
        row->owner = runtime_file_dialog_request_new(runtime, 0U);
        if (row->owner == NULL) {
            g_free(row);
            g_ptr_array_unref(rows);
            g_object_unref(generation);
            g_free(entries);
            return; /* Keep the previously rendered page intact. */
        }
        GtkWidget *button = gtk_button_new();
        GtkWidget *body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        GtkWidget *name = runtime_label(entries[i].name, 0.0F);
        GtkWidget *path = runtime_label(entries[i].relative_path, 0.0F);
        g_weak_ref_init(&row->list, G_OBJECT(runtime->explorer_list));
        row->generation = g_object_ref(generation);
        row->root = g_strdup(page.stats.root);
        row->path = g_strdup(entries[i].path);
        gtk_widget_add_css_class(button, "flat");
        gtk_widget_add_css_class(path, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(body), name);
        gtk_box_append(GTK_BOX(body), path);
        gtk_button_set_child(GTK_BUTTON(button), body);
        g_object_set_data_full(G_OBJECT(button), "umicom-explorer-row", row, RuntimeExplorerRowFree);
        /* Retain the existing copied path key for native automation clients. */
        g_object_set_data_full(G_OBJECT(button), "umicom-file-path", g_strdup(entries[i].path), g_free);
        g_signal_connect(button, "clicked", G_CALLBACK(runtime_on_explorer_row_clicked), row);
        g_ptr_array_add(rows, g_object_ref_sink(button));
    }
#endif

    {
        char previousGroup[UMI_FILE_INDEX_NAME_CAPACITY] = {0};

        for (size_t i = 0U; i < page.count; ++i) {
            RuntimeExplorerRow *row;
            UmiUiExplorerFilePresentation presentation;
            GtkWidget *button;
            GtkWidget *line;
            GtkWidget *icon;
            GtkWidget *body;
            GtkWidget *name;
            GtkWidget *path;
            const char *groupLabel;
            size_t nestedDepth;
            int margin;

            status = UmiUiExplorerDescribeFileIndexEntry(
                &entries[i], &presentation);
            if (status != UMI_STATUS_OK) {
                (void)memset(&presentation, 0, sizeof(presentation));
                (void)snprintf(
                    presentation.name,
                    sizeof(presentation.name),
                    "%s",
                    entries[i].name);
                (void)snprintf(
                    presentation.relativePath,
                    sizeof(presentation.relativePath),
                    "%s",
                    entries[i].relative_path);
                presentation.workspaceRoot = 1;
            }

            groupLabel = presentation.workspaceRoot
                ? "Workspace root"
                : presentation.topLevel[0] != '\0'
                    ? presentation.topLevel
                    : "Workspace root";

            if (strcmp(previousGroup, groupLabel) != 0) {
                GtkWidget *groupRow = RuntimeExplorerGroupRow(groupLabel);
                if (groupRow != NULL) {
                    g_ptr_array_add(rows, g_object_ref_sink(groupRow));
                }
                runtime_copy_selection(
                    previousGroup,
                    sizeof(previousGroup),
                    groupLabel);
            }

            row = g_new0(RuntimeExplorerRow, 1);
            row->owner = runtime_file_dialog_request_new(runtime, 0U);
            if (row->owner == NULL) {
                g_free(row);
                g_ptr_array_unref(rows);
                g_object_unref(generation);
                g_free(entries);
                return; /* Keep the previously rendered page intact. */
            }

            button = gtk_button_new();
            line = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            icon = gtk_image_new_from_icon_name("text-x-generic-symbolic");
            body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
            name = runtime_label(presentation.name, 0.0F);
            path = runtime_label(
                presentation.parentPath[0] != '\0'
                    ? presentation.parentPath
                    : "Workspace root",
                0.0F);

            g_weak_ref_init(&row->list, G_OBJECT(runtime->explorer_list));
            row->generation = g_object_ref(generation);
            row->root = g_strdup(page.stats.root);
            row->path = g_strdup(entries[i].path);

            nestedDepth = presentation.depth > 0U
                ? presentation.depth - 1U
                : 0U;
            if (nestedDepth > 8U) nestedDepth = 8U;
            margin = (int)(nestedDepth * 12U);

            gtk_widget_add_css_class(button, "flat");
            gtk_widget_add_css_class(path, "umicom-runtime-muted");
            gtk_widget_set_margin_start(button, margin);
            gtk_widget_set_tooltip_text(button, entries[i].path);
            gtk_widget_set_hexpand(body, TRUE);
            gtk_box_append(GTK_BOX(body), name);
            gtk_box_append(GTK_BOX(body), path);
            gtk_box_append(GTK_BOX(line), icon);
            gtk_box_append(GTK_BOX(line), body);
            gtk_button_set_child(GTK_BUTTON(button), line);

            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-explorer-row",
                row,
                RuntimeExplorerRowFree);
            /* Retain the existing copied path key for native automation clients. */
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-file-path",
                g_strdup(entries[i].path),
                g_free);
            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-file-relative-path",
                g_strdup(presentation.relativePath),
                g_free);
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_explorer_row_clicked),
                row);
            g_ptr_array_add(rows, g_object_ref_sink(button));
        }
    }

    /* Publish only after every row is ready; a failed allocation keeps the old page. */
    g_object_set_data_full(G_OBJECT(runtime->explorer_list), "umicom-explorer-rows",
        generation, g_object_unref);
    runtime_clear_list_box(runtime->explorer_list);
    for (guint i = 0U; i < rows->len; ++i)
        gtk_list_box_append(GTK_LIST_BOX(runtime->explorer_list), g_ptr_array_index(rows, i));
    g_ptr_array_unref(rows);
    if (page.count == 0U)
        gtk_list_box_append(GTK_LIST_BOX(runtime->explorer_list),
            runtime_label(page.stats.files == 0U ? "No files are indexed. Create a project file or refresh." :
                "No files match this filter.", 0.0F));

    /*
     * Keep the previous summary wording for review. The active summary adds
     * the workspace name so the Explorer remains clear when several Studio
     * windows are open.
     */
#if 0
    (void)snprintf(summary, sizeof(summary), "%zu–%zu of %zu matching files (%zu indexed)",
        page.count == 0U ? 0U : page.offset + 1U, page.offset + page.count,
        page.matched, page.stats.files);
#endif
    workspaceName[0] = '\0';
    if (umi_path_basename(
            page.stats.root,
            workspaceName,
            sizeof(workspaceName)) != UMI_STATUS_OK ||
        workspaceName[0] == '\0') {
        (void)snprintf(
            workspaceName,
            sizeof(workspaceName),
            "%s",
            "Workspace");
    }
    (void)snprintf(
        summary,
        sizeof(summary),
        "%.64s · %zu–%zu of %zu matching files (%zu indexed)",
        workspaceName,
        page.count == 0U ? 0U : page.offset + 1U,
        page.offset + page.count,
        page.matched,
        page.stats.files);

    if (runtime->explorer_summary_label != NULL)
        gtk_label_set_text(GTK_LABEL(runtime->explorer_summary_label), summary);
    if (runtime->explorer_previous != NULL)
        gtk_widget_set_sensitive(runtime->explorer_previous, page.offset != 0U);
    if (runtime->explorer_next != NULL)
        gtk_widget_set_sensitive(runtime->explorer_next, page.has_more);
    runtime->explorer_revision = page.stats.revision;
    runtime->explorer_rendered_offset = page.offset;
    runtime_copy_selection(runtime->explorer_rendered_query,
        sizeof(runtime->explorer_rendered_query), runtime->explorer_query);
    runtime->explorer_rendered = 1;
    g_free(entries);
}

/* Page numbers belong to this presentation; filtering and row ownership stay
 * in the Framework index. Buttons never trigger a filesystem scan. */
static void RuntimeExplorerPrevious(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    (void)button;
    runtime->explorer_offset = runtime->explorer_offset > UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT
        ? runtime->explorer_offset - UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT : 0U;
    runtime_explorer_refresh_list(runtime);
}

static void RuntimeExplorerNext(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    (void)button;
    if (runtime->explorer_offset <= SIZE_MAX - UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT)
        runtime->explorer_offset += UMI_STUDIO_GTK_EXPLORER_ROW_LIMIT;
    runtime_explorer_refresh_list(runtime);
}

/* Keep created files when a later editor/index operation is unavailable. The
 * developer can refresh or open that file without repeating its creation. */
static UmiStatus RuntimeCreateProjectEntry(UmiStudioGtkRuntimeChrome *runtime,
    const char *path, UmiWorkspaceEntryKind kind)
{
    UmiStudioUi *ui = runtime->owner->ui;
    UmiWorkspaceGraphSnapshot workspace;
    UmiStudioProjectEntryResult result;
    UmiStatus status = umi_workspace_graph_snapshot(umi_studio_services_workspace(
        umi_studio_ui_services(ui)), &workspace);
    if (status != UMI_STATUS_OK) return status;
    status = UmiStudioUiCreateProjectEntry(ui, workspace.revision, path, kind, &result);
    if (status != UMI_STATUS_OK) return status;
    if (kind == UMI_WORKSPACE_ENTRY_DIRECTORY)
        runtime_show_notification(runtime, "Folder created. Use its relative path when creating a project file; Explorer lists files, not empty folders.");
    else if (result.document_status != UMI_STATUS_OK || result.index_status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "File created on disk, but the index or editor could not adopt it. Refresh or use Open File; do not recreate it.");
    else runtime_show_notification(runtime, "Project file created and opened. Edit it, then save before building.");
    (void)runtime_refresh_services(runtime);
    return UMI_STATUS_OK;
}

static UmiStatus RuntimeCreateProjectFile(UmiStudioGtkRuntimeChrome *runtime, const char *path)
{ return RuntimeCreateProjectEntry(runtime, path, UMI_WORKSPACE_ENTRY_FILE); }

static UmiStatus RuntimeCreateProjectFolder(UmiStudioGtkRuntimeChrome *runtime, const char *path)
{ return RuntimeCreateProjectEntry(runtime, path, UMI_WORKSPACE_ENTRY_DIRECTORY); }

static void RuntimeExplorerNewFile(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    (void)button;
    UmiStatus status = runtime_prompt_text_internal(runtime, "New Project File",
        "Enter a path relative to the workspace. Existing files are never replaced.",
        "src/notes.h", "Create File", RuntimeCreateProjectFile, 1);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
        "Open a workspace and review its trust in Build > Project Settings before creating project files.");
}

static void RuntimeExplorerNewFolder(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    (void)button;
    UmiStatus status = runtime_prompt_text_internal(runtime, "New Project Folder",
        "Enter a relative folder path. Its parent must already exist.",
        "include", "Create Folder", RuntimeCreateProjectFolder, 1);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime,
        "Open a workspace and review its trust in Build > Project Settings before creating folders.");
}

/*
 * Provide the runtime on explorer search changed operation used by this module and its
 * client applications.
 */
static void runtime_on_explorer_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *text;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    text = gtk_editable_get_text(GTK_EDITABLE(entry));
    runtime_copy_selection(
        runtime->explorer_query,
        sizeof(runtime->explorer_query),
        text);
    runtime->explorer_offset = 0U;
    runtime_explorer_refresh_list(runtime);
}

/*
 * Provide the runtime on explorer refresh operation used by this module and its client
 * applications.
 */
static void runtime_on_explorer_refresh(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStatus status;
    (void)button;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL) return;

    status = UmiStudioUiRefreshProjectFiles(runtime->owner->ui);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        runtime_show_notification(runtime, umi_status_text(status));
    }

    runtime_explorer_refresh_list(runtime);
}

/* Control bindings borrow a live list and resolve the existing window token.
 * A widget retained after layout replacement cannot address a new Explorer. */
typedef struct RuntimeExplorerControl {
    UmiStudioFileDialogRequest *owner;
    GWeakRef list;
    void (*invoke)(gpointer widget, UmiStudioGtkRuntimeChrome *runtime);
} RuntimeExplorerControl;

static void RuntimeExplorerControlFree(gpointer data)
{
    RuntimeExplorerControl *control = data;
    runtime_file_dialog_request_free(control->owner);
    g_weak_ref_clear(&control->list);
    g_free(control);
}

static void RuntimeExplorerControlInvoked(gpointer widget, gpointer data)
{
    RuntimeExplorerControl *control = data;
    GObject *window = NULL;
    GObject *list = g_weak_ref_get(&control->list);
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(control->owner, &window);
    if (runtime != NULL && list != NULL && runtime->explorer_list == GTK_WIDGET(list))
        control->invoke(widget, runtime);
    g_clear_object(&list);
    g_clear_object(&window);
}

static void RuntimeExplorerBindControl(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *widget, const char *signal, const char *id,
    void (*invoke)(gpointer, UmiStudioGtkRuntimeChrome *))
{
    RuntimeExplorerControl *control = g_new0(RuntimeExplorerControl, 1);
    control->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (control->owner == NULL) {
        g_free(control);
        gtk_widget_set_sensitive(widget, FALSE);
        return;
    }
    g_weak_ref_init(&control->list, G_OBJECT(runtime->explorer_list));
    control->invoke = invoke;
    g_object_set_data_full(G_OBJECT(widget), "umicom-explorer-control", control,
        RuntimeExplorerControlFree);
    g_object_set_data_full(G_OBJECT(widget), "umicom-automation-id", g_strdup(id), g_free);
    g_signal_connect(widget, signal, G_CALLBACK(RuntimeExplorerControlInvoked), control);
}

static void RuntimeExplorerPreviousInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ RuntimeExplorerPrevious(GTK_BUTTON(widget), runtime); }
static void RuntimeExplorerNextInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ RuntimeExplorerNext(GTK_BUTTON(widget), runtime); }
static void RuntimeExplorerNewFileInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ RuntimeExplorerNewFile(GTK_BUTTON(widget), runtime); }
static void RuntimeExplorerNewFolderInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ RuntimeExplorerNewFolder(GTK_BUTTON(widget), runtime); }
static void RuntimeExplorerRefreshInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ runtime_on_explorer_refresh(GTK_BUTTON(widget), runtime); }
static void RuntimeExplorerCancelRefreshInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{
    (void)widget;
    UmiStatus status = UmiStudioUiCancelProjectFileRefresh(runtime->owner->ui);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
    runtime_explorer_refresh_list(runtime);
}
static void RuntimeExplorerSearchInvoked(gpointer widget, UmiStudioGtkRuntimeChrome *runtime)
{ runtime_on_explorer_search_changed(GTK_SEARCH_ENTRY(widget), runtime); }

/* Clearing a destroyed list's borrowed fields prevents later status refreshes
 * from following stale GTK addresses. The token outlives the runtime safely. */
typedef struct RuntimeExplorerListOwner {
    UmiStudioFileDialogRequest *owner;
    GtkWidget *list;
} RuntimeExplorerListOwner;

static void RuntimeExplorerListReleased(gpointer data)
{
    RuntimeExplorerListOwner *binding = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    if (runtime != NULL && runtime->explorer_list == binding->list) {
        runtime->explorer_list = NULL;
        runtime->explorer_search_entry = NULL;
        runtime->explorer_summary_label = NULL;
        runtime->explorer_refresh_status = NULL;
        runtime->explorer_refresh_button = NULL;
        runtime->explorer_cancel_refresh = NULL;
        runtime->explorer_previous = NULL;
        runtime->explorer_next = NULL;
        runtime->explorer_rendered = 0;
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
