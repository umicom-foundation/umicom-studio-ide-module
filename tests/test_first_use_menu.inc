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
    /* Captions now share their row with the real keyboard binding. The old
     * single-label assertion is retained for the previous presentation. */
#if 0
    CHECK(GTK_IS_LABEL(caption) && gtk_label_get_xalign(GTK_LABEL(caption)) == 0.0F);
#endif
    caption = gtk_widget_get_first_child(caption);
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


/* Inspect real menu children rather than assuming profile order proves native
 * grouping. Separators must never interrupt a task or strand an empty group. */
static int ExerciseMenuDetails(UmiStudioGtkWorkbench *workbench,
    UmiDocumentCoordinator *documents, const char *mode)
{
    int failed = 0;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *root = umi_gtk4_automation_find_tagged_widget(window, "umicom.shell.file-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(root));
    GtkWidget *popover = GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(root)));
    GtkWidget *newFile = FindMenuCommand(popover, "file.new");
    GtkWidget *saveAs = FindMenuCommand(popover, "file.save-as");
    CHECK(GTK_IS_BUTTON(newFile) && GTK_IS_BUTTON(saveAs));
    if (strcmp(mode, "menu-task-groups") == 0) {
        const char *starts[] = {"file.save", "file.close", "build.install", "application.exit"};
        GtkWidget *list = gtk_widget_get_parent(newFile);
        unsigned separators = 0U, rows = 0U;
        gboolean previousSeparator = FALSE;
        for (GtkWidget *row = gtk_widget_get_first_child(list); row != NULL;
             row = gtk_widget_get_next_sibling(row)) {
            if (GTK_IS_SEPARATOR(row)) {
                CHECK(rows != 0U && !previousSeparator);
                CHECK(gtk_widget_get_next_sibling(row) != NULL);
                ++separators;
                previousSeparator = TRUE;
            } else {
                CHECK(GTK_IS_BUTTON(row));
                ++rows;
                previousSeparator = FALSE;
            }
        }
        CHECK(separators == 4U && rows == 21U);
        for (size_t i = 0U; i < sizeof starts / sizeof starts[0]; ++i) {
            GtkWidget *first = FindMenuCommand(popover, starts[i]);
            CHECK(GTK_IS_BUTTON(first));
            CHECK(GTK_IS_SEPARATOR(gtk_widget_get_prev_sibling(first)));
        }
    } else {
        const char *commands[] = {"file.new", "file.open", "file.save", "file.save-as"};
        const guint keys[] = {GDK_KEY_n, GDK_KEY_o, GDK_KEY_s, GDK_KEY_s};
        const GdkModifierType modifiers[] = {GDK_CONTROL_MASK, GDK_CONTROL_MASK,
            GDK_CONTROL_MASK, GDK_CONTROL_MASK | GDK_SHIFT_MASK};
        for (size_t i = 0U; i < sizeof commands / sizeof commands[0]; ++i) {
            GtkWidget *button = FindMenuCommand(popover, commands[i]);
            CHECK(GTK_IS_BUTTON(button));
            GtkWidget *row = gtk_button_get_child(GTK_BUTTON(button));
            CHECK(GTK_IS_BOX(row));
            GtkWidget *caption = gtk_widget_get_first_child(row);
            CHECK(GTK_IS_LABEL(caption));
            GtkWidget *shortcut = gtk_widget_get_next_sibling(caption);
            CHECK(GTK_IS_SHORTCUT_LABEL(shortcut));
            guint key = 0U;
            GdkModifierType mods = 0;
            CHECK(gtk_accelerator_parse(
                gtk_shortcut_label_get_accelerator(GTK_SHORTCUT_LABEL(shortcut)), &key, &mods));
            CHECK(key == keys[i] && mods == modifiers[i]);
        }
    }
    /* Save As is a document command. A caption must not make it executable
     * before a source exists, nor remain disabled after native New File. */
    CHECK(!gtk_widget_get_sensitive(saveAs));
    CHECK(umi_document_coordinator_count(documents) == 0U);
    g_signal_emit_by_name(newFile, "clicked");
    CHECK(umi_document_coordinator_count(documents) == 1U);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    CHECK(FindMenuCommand(popover, "file.save-as") == saveAs);
    CHECK(gtk_widget_get_sensitive(saveAs));
cleanup:
    return failed;
}
