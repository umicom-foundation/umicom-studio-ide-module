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


/* Compare every rendered top-level menu against the Framework persona.
 * This proves wiring and presentation, not that an external service succeeds.
 * Destructive or credential-dependent commands need their own isolated fixture. */
static int ExerciseAllMenuBindings(UmiStudioGtkWorkbench *workbench)
{
    int failed = 0;
    UmiApplicationShellRegistry *registry = NULL;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    const UmiApplicationShellPersonaDefinition *persona =
        umi_application_shell_persona_find("umicom.shell.persona.studio-ide");
    CHECK(persona != NULL);
    CHECK(umi_application_shell_registry_create(&registry) == UMI_STATUS_OK);
    CHECK(umi_application_shell_persona_install(registry, persona) == UMI_STATUS_OK);
    size_t roots = 0U, commands = 0U;
    for (size_t i = 0U; i < umi_application_shell_registry_count(registry); ++i) {
        UmiApplicationShellContribution menu;
        CHECK(umi_application_shell_registry_at(registry, i, &menu) == UMI_STATUS_OK);
        if (menu.role != UMI_APPLICATION_SHELL_ROLE_MENU ||
            menu.region != UMI_APPLICATION_SHELL_REGION_MENU_BAR ||
            !(menu.flags & UMI_APPLICATION_SHELL_VISIBLE)) continue;
        ++roots;
        GtkWidget *root = umi_gtk4_automation_find_tagged_widget(window, menu.contribution_id);
        CHECK(GTK_IS_MENU_BUTTON(root));
        GtkWidget *popover = GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(root)));
        CHECK(GTK_IS_POPOVER(popover));
        const char *suffix = strrchr(menu.contribution_id, '.');
        CHECK(suffix != NULL);
        size_t prefix = (size_t)(suffix - menu.contribution_id);
        for (size_t j = 0U; j < umi_application_shell_registry_count(registry); ++j) {
            UmiApplicationShellContribution item;
            CHECK(umi_application_shell_registry_at(registry, j, &item) == UMI_STATUS_OK);
            if (item.role != UMI_APPLICATION_SHELL_ROLE_MENU_ITEM ||
                !(item.flags & UMI_APPLICATION_SHELL_VISIBLE) ||
                strncmp(item.contribution_id, menu.contribution_id, prefix) != 0 ||
                item.contribution_id[prefix] != '.') continue;
            GtkWidget *button =
                umi_gtk4_automation_find_tagged_widget(popover, item.contribution_id);
            CHECK(GTK_IS_BUTTON(button));
            CHECK(g_strcmp0(g_object_get_data(G_OBJECT(button), "umicom-command-id"),
                item.command_id) == 0);
            CHECK(g_strcmp0(g_object_get_data(G_OBJECT(button), "umicom-contribution-id"),
                item.contribution_id) == 0);
            CHECK(ContainsLabel(button, item.title));
            CHECK(g_signal_has_handler_pending(button,
                g_signal_lookup("clicked", GTK_TYPE_BUTTON), 0, TRUE));
            /* Emit a compact coverage record, including deliberately unavailable
             * actions. An unavailable command must not be counted as executed. */
            fprintf(stdout, "menu-binding\t%s\t%s\t%s\n", menu.title, item.command_id,
                gtk_widget_get_sensitive(button) ? "enabled" : "unavailable-in-empty-workspace");
            ++commands;
        }
    }
    CHECK(roots == 11U && commands >= 103U);
cleanup:
    umi_application_shell_registry_destroy(registry);
    return failed;
}

/* A click must change the authoritative panel model. Each case begins on a
 * blank canvas so an already visible default panel cannot mask a lost event. */
static int ExerciseMenuPanel(UmiStudioGtkWorkbench *workbench, const char *name)
{
    int failed = 0;
    UmiUiWorkspaceLayout *layout = g_new0(UmiUiWorkspaceLayout, 1);
    const char *command = "help.documentation";
    UmiStudioRuntimeSurfaceKind kind = UMI_STUDIO_SURFACE_DOCUMENTATION;
    if (!strcmp(name, "menu-learning-panel")) {
        command = "help.ai-teacher"; kind = UMI_STUDIO_SURFACE_LEARNING;
    } else if (!strcmp(name, "menu-search-panel")) {
        command = "view.search"; kind = UMI_STUDIO_SURFACE_SEARCH;
    } else if (!strcmp(name, "menu-live-preview-panel")) {
        command = "view.live-preview"; kind = UMI_STUDIO_SURFACE_LIVE_PREVIEW;
    } else CHECK(!strcmp(name, "menu-documentation-panel"));
    CHECK(umi_studio_gtk_workbench_workspace_create_blank(workbench,
        "umicom.studio.layout.menu-check", "Menu check") == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_commit_edit(workbench) == UMI_STATUS_OK);
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *button = FindMenuCommand(window, command);
    CHECK(GTK_IS_BUTTON(button) && gtk_widget_get_sensitive(button));
    g_signal_emit_by_name(button, "clicked");
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    const char *id = umi_studio_runtime_workspace_surface_id(kind);
    const UmiUiWorkspaceWindow *panel = umi_ui_workspace_layout_find_window(layout, id);
    CHECK(panel != NULL && panel->visible);
    /* Repeating an ordinary Open focuses the same singleton rather than
     * producing duplicate panels or replacing the document model. */
    size_t panelCount = layout->window_count;
    g_signal_emit_by_name(button, "clicked");
    CHECK(umi_studio_gtk_workbench_workspace_snapshot(workbench, layout) == UMI_STATUS_OK);
    CHECK(layout->window_count == panelCount);
    size_t matches = 0U;
    for (size_t i = 0U; i < layout->window_count; ++i)
        if (!strcmp(layout->windows[i].window_id, id)) ++matches;
    CHECK(matches == 1U);
cleanup:
    g_free(layout);
    return failed;
}

/* Menu activation must present the real owned palette. Closing and reopening
 * it must leave the Studio owner available, rather than strand a modal window. */
static int ExerciseMenuPalette(UmiStudioGtkWorkbench *workbench)
{
    int failed = 0;
    GtkWindow *palette = NULL;
    GtkWidget *window = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *button = FindMenuCommand(window, "workbench.command-palette");
    CHECK(GTK_IS_BUTTON(button) && gtk_widget_get_sensitive(button));
    g_signal_emit_by_name(button, "clicked");
    GListModel *windows = gtk_window_get_toplevels();
    for (guint i = 0U; i < g_list_model_get_n_items(windows); ++i) {
        GtkWindow *candidate = g_list_model_get_item(windows, i);
        if (gtk_window_get_transient_for(candidate) == GTK_WINDOW(window) &&
            !g_strcmp0(gtk_window_get_title(candidate), "Search Commands and Menus")) {
            palette = candidate; break;
        }
        g_object_unref(candidate);
    }
    CHECK(palette != NULL && gtk_widget_get_visible(GTK_WIDGET(palette)));
    CHECK(gtk_window_get_modal(palette));
    gtk_window_close(palette);
    CHECK(!gtk_widget_get_visible(GTK_WIDGET(palette)));
    g_signal_emit_by_name(button, "clicked");
    CHECK(gtk_widget_get_visible(GTK_WIDGET(palette)));
    gtk_window_close(palette);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
cleanup:
    g_clear_object(&palette);
    return failed;
}
