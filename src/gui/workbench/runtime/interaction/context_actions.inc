/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/context_actions.inc
 *
 * PURPOSE:
 *   Render the complete canonical Studio command inventory contextually inside
 *   professional tool panels. Profiles only place commands; the real command
 *   registry remains authoritative for existence, enablement and execution.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_context_action(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id;
    const UmiStudioGtkOperationProfile *operation;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    command_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-context-command");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id == NULL) return;

    operation = runtime_operation_profile(command_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operation == NULL) {
        runtime_show_notification(
            runtime,
            "Context action operation profile is unavailable.");
        return;
    }

    (void)runtime_execute_operation_profile(runtime, operation);
}

/*
 * Provide the runtime context actions expander operation used by this module and its
 * client applications.
 */
static GtkWidget *runtime_context_actions_expander(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *panel_id)
{
    UmiCommandRegistry *commands;
    GtkWidget *expander;
    GtkWidget *flow;
    size_t index;
    size_t added = 0U;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || panel_id == NULL) return NULL;

    commands = runtime_registered_commands(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands == NULL) return NULL;

    expander = gtk_expander_new("Advanced Actions");
    flow = gtk_flow_box_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (expander == NULL || flow == NULL) return expander;

    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow),
        GTK_SELECTION_NONE);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 4U);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 4U);
    gtk_flow_box_set_max_children_per_line(
        GTK_FLOW_BOX(flow),
        4U);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < runtime_context_action_profile_count();
         ++index) {
        const UmiStudioGtkContextActionProfile *context =
            &RUNTIME_CONTEXT_ACTION_PROFILES[index];
        const UmiStudioGtkOperationProfile *operation;
        GtkWidget *button;
        int sensitive;

        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (strcmp(context->panel_id, panel_id) != 0) continue;
        /* Apply this branch only when its contract condition is satisfied. */
        if (!umi_command_registry_contains(
                commands,
                context->command_id)) {
            continue;
        }

        operation = runtime_operation_profile(context->command_id);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (operation == NULL) continue;

        button = gtk_button_new_with_label(operation->title);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button == NULL) continue;

        gtk_widget_set_tooltip_text(
            button,
            operation->description);
        gtk_widget_add_css_class(button, "flat");

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-context-command",
            g_strdup(operation->command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_context_action),
            runtime);

        sensitive =
            operation->argument_kind ==
                UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE
                ? umi_command_registry_is_enabled(
                    commands,
                    operation->command_id,
                    NULL)
                : 1;
        gtk_widget_set_sensitive(button, sensitive);

        gtk_flow_box_append(GTK_FLOW_BOX(flow), button);
        ++added;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (added == 0U) {
        gtk_widget_set_visible(expander, FALSE);
    }

    gtk_expander_set_child(GTK_EXPANDER(expander), flow);
    return expander;
}

/*
 * Provide the runtime append context actions operation used by this module and its client
 * applications.
 */
static void runtime_append_context_actions(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *panel,
    const char *panel_id)
{
    GtkWidget *expander;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || panel == NULL || panel_id == NULL) return;

    expander = runtime_context_actions_expander(runtime, panel_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (expander != NULL) {
        gtk_box_append(GTK_BOX(panel), expander);
    }
}
