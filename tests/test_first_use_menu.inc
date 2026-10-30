/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_first_use_menu.inc
 *
 * PURPOSE:
 *   Exercise visible menu order, bounded presentation and native New File activation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Read actual GTK children so a correctly sorted model cannot hide an incorrectly
 * ordered native menu. This test also invokes New File through that menu. */
static int ExerciseMenuPresentation(UmiStudioGtkWorkbench *workbench,
    UmiDocumentCoordinator *documents)
{
    int failed = 0;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *root = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.file-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(root));
    GtkWidget *popover = GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(root)));
    CHECK(GTK_IS_POPOVER(popover));
    GtkWidget *scroll = gtk_popover_get_child(GTK_POPOVER(popover));
    CHECK(GTK_IS_SCROLLED_WINDOW(scroll));
    CHECK(gtk_scrolled_window_get_max_content_height(GTK_SCROLLED_WINDOW(scroll)) == 480);
    GtkWidget *first = FindAction(popover, "project.new");
    GtkWidget *newFile = FindAction(popover, "file.new");
    GtkWidget *open = FindAction(popover, "file.open");
    GtkWidget *save = FindAction(popover, "file.save");
    CHECK(first != NULL && newFile != NULL && open != NULL && save != NULL);
    GtkWidget *list = gtk_widget_get_parent(first);
    CHECK(gtk_widget_get_first_child(list) == first);
    CHECK(gtk_widget_get_next_sibling(first) == newFile);
    CHECK(gtk_widget_get_next_sibling(newFile) == open);
    gboolean sawSave = FALSE;
    for (GtkWidget *row = open; row != NULL; row = gtk_widget_get_next_sibling(row))
        if (row == save) sawSave = TRUE;
    CHECK(sawSave && gtk_widget_get_sensitive(newFile));
    GtkWidget *caption = gtk_button_get_child(GTK_BUTTON(newFile));
    CHECK(GTK_IS_LABEL(caption) && gtk_label_get_xalign(GTK_LABEL(caption)) == 0.0F);
    CHECK(umi_document_coordinator_count(documents) == 0U);
    g_signal_emit_by_name(newFile, "clicked");
    CHECK(umi_document_coordinator_count(documents) == 1U);
    UmiDocumentWorkingCopySnapshot active;
    CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
    GtkWidget *editor = FindEditor(window, active.view_id);
    CHECK(GTK_IS_TEXT_VIEW(editor) && gtk_text_view_get_editable(GTK_TEXT_VIEW(editor)));
cleanup:
    return failed;
}
