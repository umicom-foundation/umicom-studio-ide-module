/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/explorer/project_explorer_view.c
 *
 * PURPOSE:
 *   Render workspace identity, trust and indexed-file counts from Framework
 *   services without performing filesystem operations in the widget.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "project_explorer_view.h"

GtkWidget *umi_studio_project_explorer_view_new(UmiStudioUi *ui)
{
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *summary;
    UmiWorkspaceGraphSnapshot workspace = {0};
    UmiFileIndexStats files;
    UmiStudioServices *services;
    const char *root_text;
    gchar *summary_text;

    if (ui == NULL) {
        return NULL;
    }

    services = umi_studio_ui_services(ui);
    files = umi_file_index_stats(
        umi_studio_services_file_index(services));
    (void)umi_workspace_graph_snapshot(
        umi_studio_services_workspace(services),
        &workspace);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    title = gtk_label_new("Project Explorer");
    gtk_widget_add_css_class(title, "title-3");

    /*
     * Workspace roots are deliberately larger than the old fixed presentation
     * buffer. Let GLib allocate the exact display string instead of silently
     * truncating a valid path or relying on compiler-warning suppression.
     */
    root_text = workspace.root[0] != '\0'
        ? workspace.root
        : "No workspace";
    summary_text = g_strdup_printf(
        "Root: %s\nProjects: %zu\nFiles: %zu\nTrusted: %s",
        root_text,
        workspace.project_count,
        files.files,
        workspace.trusted ? "yes" : "no");

    summary = gtk_label_new(
        summary_text != NULL
            ? summary_text
            : "Project summary unavailable");
    g_free(summary_text);

    gtk_label_set_xalign(GTK_LABEL(summary), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(summary), TRUE);
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), summary);
    return box;
}
