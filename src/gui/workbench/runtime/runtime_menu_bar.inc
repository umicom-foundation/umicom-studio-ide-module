/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_menu_bar.inc
 *
 * PURPOSE:
 *   Render the Framework Studio Application Shell persona as a native GTK4 IDE
 *   menu strip. Studio does not duplicate File/Edit/View/etc. inventories.
 *
 * COMMAND POLICY:
 *   Item sensitivity comes from the reconciled Application Shell contribution.
 *   Clicking an item executes that contribution through the Framework Studio
 *   Runtime Host Controller and then synchronizes presentation state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Menu signals now use the same weak owner as toolbar commands, preserve
 * explicit arguments, and reacquire authority after dismissing the popover.
 * The former borrowed-runtime, chooser-only implementation is kept for review. */
#if 0
static void runtime_on_menu_item_clicked(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *contribution_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-contribution-id");
    UmiStudioRuntimeBindings *bindings;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || contribution_id == NULL ||
        runtime->runtime_bootstrap == NULL) {
        return;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->shell_registry == NULL) return;

    /*
     * Resolve the contribution back to its command identifier and route that
     * command through the same operational path used by toolbar, keyboard and
     * command-palette actions. This is what lets Open Folder, Commit and other
     * argument-requiring menu items collect native GTK input before execution.
     */
    for (index = 0U;
         index < umi_application_shell_registry_count(
             bindings->shell_registry);
         ++index) {
        UmiApplicationShellContribution contribution;

        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (strcmp(
                contribution.contribution_id,
                contribution_id) != 0) {
            continue;
        }

        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (contribution.command_id[0] == '\0') return;
        GtkWidget *popover = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_POPOVER);
        if (popover != NULL) gtk_popover_popdown(GTK_POPOVER(popover));
        (void)runtime_execute_command(
            runtime,
            contribution.command_id,
            NULL);
        return;
    }

    runtime_show_notification(runtime, "Menu contribution was not found.");
}

#endif

static void runtime_on_menu_item_clicked(GtkButton *button, gpointer user_data)
{
    UmiStudioFileDialogRequest *binding = user_data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding, &window);
    if (runtime == NULL) {
        g_clear_object(&window);
        return;
    }

    /* Dismissal can remove the button or close Studio through a GTK observer.
     * Own the selected command and argument before invoking any such observer. */
    const char *id = g_object_get_data(G_OBJECT(button), "umicom-contribution-id");
    UmiStudioRuntimeBindings *bindings = runtime->runtime_bootstrap != NULL
        ? umi_studio_runtime_platform_bindings(runtime->runtime_bootstrap->platform) : NULL;
    char command[UMI_APPLICATION_SHELL_ID_CAPACITY] = {0};
    if (id != NULL && bindings != NULL && bindings->shell_registry != NULL) {
        for (size_t index = 0U; index < umi_application_shell_registry_count(bindings->shell_registry); ++index) {
            UmiApplicationShellContribution contribution;
            if (umi_application_shell_registry_at(bindings->shell_registry, index,
                    &contribution) == UMI_STATUS_OK &&
                strcmp(contribution.contribution_id, id) == 0) {
                g_strlcpy(command, contribution.command_id, sizeof command);
                break;
            }
        }
    }
    if (command[0] == '\0') {
        runtime_show_notification(runtime, "Menu contribution was not found.");
        g_clear_object(&window);
        return;
    }

    char *argument = g_strdup(g_object_get_data(G_OBJECT(button), "umicom-command-argument"));
    /* Use an independent request for the rest of this call: popdown may release
     * the signal closure which owns binding before control returns here. */
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) {
        g_free(argument);
        g_clear_object(&window);
        return;
    }
    GtkWidget *popover = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_POPOVER);
    if (popover != NULL) {
        g_object_ref(popover);
        gtk_popover_popdown(GTK_POPOVER(popover));
        g_object_unref(popover);
    }
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) (void)runtime_execute_command(runtime, command, argument);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(argument);
}

/*
 * Provide the runtime contribution has prefix operation used by this module and its client
 * applications.
 */
static int runtime_contribution_has_prefix(
    const char *contribution_id,
    const char *prefix)
{
    size_t prefix_length;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (contribution_id == NULL || prefix == NULL) return 0;
    prefix_length = strlen(prefix);
    return strncmp(contribution_id, prefix, prefix_length) == 0;
}

/*
 * Provide the runtime build single menu operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_build_single_menu(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiApplicationShellContribution *root)
{
    UmiStudioRuntimeBindings *bindings;
    GtkWidget *menu_button;
    GtkWidget *popover;
    GtkWidget *items;
    char prefix[UMI_APPLICATION_SHELL_ID_CAPACITY];
    const char *root_suffix;
    size_t prefix_length;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || root == NULL ||
        runtime->runtime_bootstrap == NULL) {
        return NULL;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->shell_registry == NULL) return NULL;

    root_suffix = strstr(root->contribution_id, ".root");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root_suffix == NULL) return NULL;

    prefix_length = (size_t)(root_suffix - root->contribution_id);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (prefix_length == 0U || prefix_length >= sizeof(prefix)) return NULL;

    (void)memcpy(prefix, root->contribution_id, prefix_length);
    prefix[prefix_length] = '\0';

    menu_button = gtk_menu_button_new();
    popover = gtk_popover_new();
    items = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (menu_button == NULL || popover == NULL || items == NULL) {
        return menu_button;
    }

    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), root->title);
    /* A menu root keeps its contribution ID across themes and translations. */
    (void)umi_gtk4_automation_tag_widget(
        menu_button,
        root->contribution_id);
    gtk_widget_add_css_class(menu_button, "flat");

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_application_shell_registry_count(
             bindings->shell_registry);
         ++index) {
        UmiApplicationShellContribution contribution;
        GtkWidget *button;

        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (contribution.role != UMI_APPLICATION_SHELL_ROLE_MENU_ITEM ||
            !runtime_contribution_has_prefix(
                contribution.contribution_id,
                prefix)) {
            continue;
        }

        button = gtk_button_new_with_label(contribution.title);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button == NULL) continue;

        gtk_widget_set_halign(button, GTK_ALIGN_FILL);
        gtk_widget_add_css_class(button, "flat");
        gtk_widget_set_tooltip_text(button, contribution.description);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, contribution.command_id));
        g_object_set_data_full(G_OBJECT(button), "umicom-command-id",
            g_strdup(contribution.command_id), g_free);

        /* Menu rows use their unique contribution ID to avoid ambiguous matches. */
        (void)umi_gtk4_automation_tag_widget(
            button,
            contribution.contribution_id);

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-contribution-id",
            g_strdup(contribution.contribution_id),
            g_free);
        /* Retained menu controls must lose authority when their Studio owner
         * retires. The earlier direct binding is preserved below for review. */
#if 0
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_menu_item_clicked),
            runtime);
#endif
        if (!RuntimeConnectCommandButton(runtime, button,
                G_CALLBACK(runtime_on_menu_item_clicked)))
            gtk_widget_set_sensitive(button, FALSE);

        gtk_box_append(GTK_BOX(items), button);
    }

    gtk_widget_set_margin_top(items, 5);
    gtk_widget_set_margin_bottom(items, 5);
    gtk_widget_set_margin_start(items, 5);
    gtk_widget_set_margin_end(items, 5);
    gtk_popover_set_child(GTK_POPOVER(popover), items);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(menu_button), popover);
    return menu_button;
}

/*
 * Provide the runtime build menu bar operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_build_menu_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioRuntimeBindings *bindings;
    GtkWidget *bar;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->runtime_bootstrap == NULL) return NULL;

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->shell_registry == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bar == NULL) return NULL;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < umi_application_shell_registry_count(
             bindings->shell_registry);
         ++index) {
        UmiApplicationShellContribution contribution;
        GtkWidget *menu;

        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (contribution.role != UMI_APPLICATION_SHELL_ROLE_MENU ||
            contribution.region != UMI_APPLICATION_SHELL_REGION_MENU_BAR) {
            continue;
        }

        menu = runtime_build_single_menu(runtime, &contribution);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (menu != NULL) gtk_box_append(GTK_BOX(bar), menu);
    }

    return bar;
}
