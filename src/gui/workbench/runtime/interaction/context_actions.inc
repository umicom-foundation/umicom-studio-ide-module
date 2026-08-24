/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/context_actions.inc
 *
 * PURPOSE:
 *   Render the complete canonical Studio command inventory contextually inside
 *   professional tool panels. Profiles only place commands; the real command
 *   registry remains authoritative for existence, enablement and execution.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_context_action(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id;
    const UmiStudioGtkOperationProfile *operation;

    if (runtime == NULL) return;

    command_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-context-command");
    if (command_id == NULL) return;

    operation = runtime_operation_profile(command_id);
    if (operation == NULL) {
        runtime_show_notification(
            runtime,
            "Context action operation profile is unavailable.");
        return;
    }

    (void)runtime_execute_operation_profile(runtime, operation);
}

static GtkWidget *runtime_context_actions_expander(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *panel_id)
{
    UmiCommandRegistry *commands;
    GtkWidget *expander;
    GtkWidget *flow;
    size_t index;
    size_t added = 0U;

    if (runtime == NULL || panel_id == NULL) return NULL;

    commands = runtime_registered_commands(runtime);
    if (commands == NULL) return NULL;

    expander = gtk_expander_new("Advanced Actions");
    flow = gtk_flow_box_new();
    if (expander == NULL || flow == NULL) return expander;

    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow),
        GTK_SELECTION_NONE);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 4U);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 4U);
    gtk_flow_box_set_max_children_per_line(
        GTK_FLOW_BOX(flow),
        4U);

    for (index = 0U;
         index < runtime_context_action_profile_count();
         ++index) {
        const UmiStudioGtkContextActionProfile *context =
            &RUNTIME_CONTEXT_ACTION_PROFILES[index];
        const UmiStudioGtkOperationProfile *operation;
        GtkWidget *button;
        int sensitive;

        if (strcmp(context->panel_id, panel_id) != 0) continue;
        if (!umi_command_registry_contains(
                commands,
                context->command_id)) {
            continue;
        }

        operation = runtime_operation_profile(context->command_id);
        if (operation == NULL) continue;

        button = gtk_button_new_with_label(operation->title);
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

    if (added == 0U) {
        gtk_widget_set_visible(expander, FALSE);
    }

    gtk_expander_set_child(GTK_EXPANDER(expander), flow);
    return expander;
}

static void runtime_append_context_actions(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *panel,
    const char *panel_id)
{
    GtkWidget *expander;

    if (runtime == NULL || panel == NULL || panel_id == NULL) return;

    expander = runtime_context_actions_expander(runtime, panel_id);
    if (expander != NULL) {
        gtk_box_append(GTK_BOX(panel), expander);
    }
}
