/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/first_use_explorer_activation.inc
 *
 * PURPOSE:
 *   Exercise visible Explorer activation and focus after native button callbacks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


/* Signal-only tests do not exercise GtkButton's delayed activation. Present an
 * isolated window and drain its main loop after each activation so focus and
 * timer callbacks complete while the test still owns the project and host. */
static int ExerciseExplorerActivation(UmiStudioGtkWorkbench *workbench,
    UmiStudioUi *ui, const char *fixture, const char *mode)
{
    int failed = 0;
    GtkWidget *button = NULL;
    GtkWindow *window = g_object_ref(umi_studio_gtk_workbench_window(workbench));
    char *folder = g_build_filename(fixture, "Explorer activation", NULL);
    char *sources[2] = {g_build_filename(folder, "first.c", NULL),
                        g_build_filename(folder, "second.c", NULL)};
    UmiStudioServices *services = umi_studio_ui_services(ui);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    CHECK(g_mkdir_with_parents(folder, 0700) == 0);
    CHECK(g_file_set_contents(sources[0], "int first = 1;\n", -1, NULL));
    CHECK(g_file_set_contents(sources[1], "int second = 2;\n", -1, NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench, "workspace.open-folder", folder) == UMI_STATUS_OK);
    CHECK(UmiFileIndexRefreshWait(umi_studio_services_file_index(services), 10000U) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EXPLORER, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    gtk_window_set_default_size(window, 1100, 740);
    gtk_window_present(window);
    gtk_window_maximize(window);
    TabJourneyPumpFor(350000);
    for (unsigned iteration = 0U; iteration < 8U; ++iteration) {
        const char *source = sources[iteration % 2U];
        GtkWidget *list = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window), "umicom-explorer-list");
        CHECK(GTK_IS_LIST_BOX(list));
        GtkWidget *candidate = ExplorerFileButton(list, source);
        CHECK(GTK_IS_BUTTON(candidate));
        button = g_object_ref(candidate);
        /* Activation can bypass native pointer routing. Check the actual hit
         * target first so an invisible drawer cannot mask a displayed row. */
        /* Focus traversal may scroll a row in a short Explorer viewport. Wait
         * for that allocation before checking input, rather than depending on
         * one fixed delay or the desktop accepting a maximise request. */
        CHECK(gtk_widget_grab_focus(button));
        graphene_rect_t bounds = GRAPHENE_RECT_INIT(0.0F, 0.0F, 0.0F, 0.0F);
        GtkWidget *picked = NULL;
        const gint64 pick_deadline = g_get_monotonic_time() + 2000000;
        do {
            TabJourneyPumpFor(25000);
            if (gtk_widget_compute_bounds(button, GTK_WIDGET(window), &bounds) &&
                bounds.size.width > 0.0F && bounds.size.height > 0.0F) {
                picked = gtk_widget_pick(GTK_WIDGET(window),
                    (double)(bounds.origin.x + bounds.size.width / 2.0F),
                    (double)(bounds.origin.y + bounds.size.height / 2.0F), GTK_PICK_DEFAULT);
                if (picked != NULL && (picked == button || gtk_widget_is_ancestor(picked, button))) break;
            }
        } while (g_get_monotonic_time() < pick_deadline);
        if (picked == NULL || (picked != button && !gtk_widget_is_ancestor(picked, button)))
            fprintf(stderr, "Explorer hit iteration=%u source=%s bounds=%g,%g,%g,%g picked=%s\n",
                iteration, source, (double)bounds.origin.x, (double)bounds.origin.y,
                (double)bounds.size.width, (double)bounds.size.height,
                picked == NULL ? "none" : G_OBJECT_TYPE_NAME(picked));
        CHECK(picked != NULL && (picked == button || gtk_widget_is_ancestor(picked, button)));
        CHECK(gtk_widget_grab_focus(button));
        CHECK(gtk_widget_get_root(button) == GTK_ROOT(window));
        if (strcmp(mode, "explorer-native-activate") == 0) {
            CHECK(gtk_widget_activate(button));
        } else {
            g_signal_emit_by_name(button, "clicked");
        }
        TabJourneyPumpFor(350000);
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
        CHECK(umi_path_equal(active.path, source));
        GtkWidget *editor = FindEditor(GTK_WIDGET(window), active.view_id);
        CHECK(Reachable(editor, GTK_WIDGET(window)));
        GtkWidget *focus = gtk_window_get_focus(window);
        CHECK(focus != NULL && (focus == editor || gtk_widget_is_ancestor(focus, editor)));
        g_clear_object(&button);
    }
    CHECK(umi_document_coordinator_count(documents) == 2U);
cleanup:
    g_clear_object(&button);
    g_clear_object(&window);
    g_free(sources[0]); g_free(sources[1]); g_free(folder);
    return failed;
}
