/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_menu.inc
 *
 * PURPOSE:
 *   Reveal existing process-attachment and breakpoint controls without starting debugger work.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* These menu actions reveal existing controls. They never submit a debugger
 * request or interpret a PID argument. Attachment remains an explicit action
 * on the established form, where Framework/Studio repeat workspace authority. */
static UmiStatus RuntimeRevealDebuggerControl(
    UmiStudioGtkRuntimeChrome *runtime, const char *command, const char *argument)
{
    int attach = command != NULL && strcmp(command, "debug.attach") == 0;
    if (runtime == NULL || runtime->owner == NULL ||
        (!attach && (command == NULL || strcmp(command, "debug.breakpoints") != 0)) ||
        (argument != NULL && argument[0] != '\0'))
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;
    GtkWidget *target = NULL, *container = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(window))) goto done;
    if (g_object_get_data(window, "umicom-debug-menu-reveal") != NULL) {
        status = UMI_STATUS_BUSY;
        goto done;
    }
    /* Retain the originating window independently of its current runtime.
     * Expanding a control or selecting a notebook page may run user observers. */
    GObject *guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-debug-menu-reveal", GINT_TO_POINTER(1));
    status = runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DEBUG);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    if (status == UMI_STATUS_OK) {
        if (attach) {
            target = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window),
                "studio.debug.attach.pid");
            container = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window),
                "studio.debug.attach.form");
            if (!GTK_IS_EDITABLE(target) || !GTK_IS_EXPANDER(container))
                status = UMI_STATUS_UNAVAILABLE;
        } else {
            target = runtime->debug_breakpoints_list;
            container = target != NULL
                ? gtk_widget_get_ancestor(target, GTK_TYPE_NOTEBOOK) : NULL;
            if (!GTK_IS_LIST_BOX(target) || !GTK_IS_NOTEBOOK(container))
                status = UMI_STATUS_UNAVAILABLE;
        }
        if (status == UMI_STATUS_OK) {
            g_object_ref(target);
            g_object_ref(container);
            if (attach) gtk_expander_set_expanded(GTK_EXPANDER(container), TRUE);
            else {
/* Framework now checks the retained page after observers, including removal
 * and competing selection. Keep the earlier unchecked selection for review. */
#if 0
                GtkWidget *page = gtk_widget_get_ancestor(target, GTK_TYPE_SCROLLED_WINDOW);
                int page_number = page != NULL
                    ? gtk_notebook_page_num(GTK_NOTEBOOK(container), page) : -1;
                if (page_number < 0) status = UMI_STATUS_UNAVAILABLE;
                else gtk_notebook_set_current_page(GTK_NOTEBOOK(container), page_number);
#endif
                status = UmiGtk4NotebookSelectOwnedTarget(GTK_WINDOW(window),
                    GTK_NOTEBOOK(container), target);
            }
            g_clear_object(&window);
            runtime = runtime_file_dialog_owner(request, &window);
            if (runtime == NULL) status = UMI_STATUS_CANCELLED;
            /* Unmapped embedders can inspect the revealed state without a
             * focus claim. In a mapped host, focus is a separate observed step. */
            if (status == UMI_STATUS_OK && gtk_widget_get_mapped(target))
                (void)gtk_widget_grab_focus(target);
            g_clear_object(&window);
            runtime = runtime_file_dialog_owner(request, &window);
            if (runtime == NULL) status = UMI_STATUS_CANCELLED;
            g_object_unref(target);
            g_object_unref(container);
        }
    }
    g_object_set_data(guard, "umicom-debug-menu-reveal", NULL);
    g_object_unref(guard);
done:
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}
