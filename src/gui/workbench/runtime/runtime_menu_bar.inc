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
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

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

    if (runtime == NULL || contribution_id == NULL ||
        runtime->runtime_bootstrap == NULL) {
        return;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
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

        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        if (strcmp(
                contribution.contribution_id,
                contribution_id) != 0) {
            continue;
        }

        if (contribution.command_id[0] == '\0') return;
        (void)runtime_route_command(
            runtime,
            contribution.command_id,
            NULL);
        return;
    }

    runtime_show_notification(runtime, "Menu contribution was not found.");
}

static int runtime_contribution_has_prefix(
    const char *contribution_id,
    const char *prefix)
{
    size_t prefix_length;

    if (contribution_id == NULL || prefix == NULL) return 0;
    prefix_length = strlen(prefix);
    return strncmp(contribution_id, prefix, prefix_length) == 0;
}

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

    if (runtime == NULL || root == NULL ||
        runtime->runtime_bootstrap == NULL) {
        return NULL;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    if (bindings == NULL || bindings->shell_registry == NULL) return NULL;

    root_suffix = strstr(root->contribution_id, ".root");
    if (root_suffix == NULL) return NULL;

    prefix_length = (size_t)(root_suffix - root->contribution_id);
    if (prefix_length == 0U || prefix_length >= sizeof(prefix)) return NULL;

    (void)memcpy(prefix, root->contribution_id, prefix_length);
    prefix[prefix_length] = '\0';

    menu_button = gtk_menu_button_new();
    popover = gtk_popover_new();
    items = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    if (menu_button == NULL || popover == NULL || items == NULL) {
        return menu_button;
    }

    gtk_menu_button_set_label(GTK_MENU_BUTTON(menu_button), root->title);
    gtk_widget_add_css_class(menu_button, "flat");

    for (index = 0U;
         index < umi_application_shell_registry_count(
             bindings->shell_registry);
         ++index) {
        UmiApplicationShellContribution contribution;
        GtkWidget *button;

        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        if (contribution.role != UMI_APPLICATION_SHELL_ROLE_MENU_ITEM ||
            !runtime_contribution_has_prefix(
                contribution.contribution_id,
                prefix)) {
            continue;
        }

        button = gtk_button_new_with_label(contribution.title);
        if (button == NULL) continue;

        gtk_widget_set_halign(button, GTK_ALIGN_FILL);
        gtk_widget_add_css_class(button, "flat");
        gtk_widget_set_tooltip_text(button, contribution.description);
        gtk_widget_set_sensitive(
            button,
            ((contribution.flags & UMI_APPLICATION_SHELL_ENABLED) != 0U) ||
            runtime_command_enabled(runtime, contribution.command_id));

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-contribution-id",
            g_strdup(contribution.contribution_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_menu_item_clicked),
            runtime);

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

static GtkWidget *runtime_build_menu_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioRuntimeBindings *bindings;
    GtkWidget *bar;
    size_t index;

    if (runtime == NULL || runtime->runtime_bootstrap == NULL) return NULL;

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    if (bindings == NULL || bindings->shell_registry == NULL) return NULL;

    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    if (bar == NULL) return NULL;

    for (index = 0U;
         index < umi_application_shell_registry_count(
             bindings->shell_registry);
         ++index) {
        UmiApplicationShellContribution contribution;
        GtkWidget *menu;

        if (umi_application_shell_registry_at(
                bindings->shell_registry,
                index,
                &contribution) != UMI_STATUS_OK) {
            continue;
        }

        if (contribution.role != UMI_APPLICATION_SHELL_ROLE_MENU ||
            contribution.region != UMI_APPLICATION_SHELL_REGION_MENU_BAR) {
            continue;
        }

        menu = runtime_build_single_menu(runtime, &contribution);
        if (menu != NULL) gtk_box_append(GTK_BOX(bar), menu);
    }

    return bar;
}
