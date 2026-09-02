/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_command_palette.inc
 *
 * PURPOSE:
 *   Render Framework command/menu search as a GTK4 command palette.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_command_palette_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *query);


/*
 * Provide the runtime on operation palette result operation used by this module and its
 * client applications.
 */
static void runtime_on_operation_palette_result(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-operation-command");
    const UmiStudioGtkOperationProfile *profile;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL) return;

    profile = runtime_operation_profile(command_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (profile == NULL) {
        runtime_show_notification(runtime, "Operation profile was not found.");
        return;
    }

    gtk_widget_set_visible(runtime->command_window, FALSE);
    (void)runtime_execute_operation_profile(runtime, profile);
}

/*
 * Provide the runtime on palette result operation used by this module and its client
 * applications.
 */
static void runtime_on_palette_result(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-id");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL) return;

    gtk_widget_set_visible(runtime->command_window, FALSE);
    (void)runtime_execute_command(runtime, command_id, NULL);
}

/*
 * Provide the runtime command palette refresh operation used by this module and its client
 * applications.
 */
static void runtime_command_palette_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *query)
{
    UmiStudioRuntimeBindings *bindings;
    UmiStudioRuntimeCommandSearchResults results;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        runtime->command_list == NULL) {
        return;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL) return;

    runtime_clear_list_box(runtime->command_list);

    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_studio_command_search(
            bindings,
            query != NULL ? query : "",
            &results) != UMI_STATUS_OK) {
        return;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < results.count; ++index) {
        const UmiStudioRuntimeCommandSearchResult *item =
            &results.items[index];
        GtkWidget *button;
        GtkWidget *box;
        GtkWidget *title;
        GtkWidget *detail;

        button = gtk_button_new();
        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        title = runtime_label(item->title, 0.0F);
        detail = runtime_label(item->command_id, 0.0F);

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button == NULL || box == NULL ||
            title == NULL || detail == NULL) {
            continue;
        }

        gtk_widget_add_css_class(button, "umicom-runtime-command-row");
        gtk_widget_add_css_class(detail, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(box), title);
        gtk_box_append(GTK_BOX(box), detail);
        gtk_button_set_child(GTK_BUTTON(button), box);

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(item->command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_palette_result),
            runtime);
        gtk_widget_set_sensitive(
            button,
            item->enabled ||
            runtime_command_enabled(runtime, item->command_id));

        gtk_list_box_append(
            GTK_LIST_BOX(runtime->command_list),
            button);
    }

    /*
     * Search Everywhere also exposes canonical Studio service commands that do
     * not have a one-to-one generic Application Shell contribution. A profile
     * is shown only when its exact command is present in UmiCommandRegistry.
     */
    {
        UmiCommandRegistry *commands = runtime_registered_commands(runtime);
        const size_t operation_count = runtime_operation_profile_count();

        /* Visit each bounded item once so every record receives the same rule. */
        for (index = 0U; index < operation_count; ++index) {
            const UmiStudioGtkOperationProfile *profile =
                &RUNTIME_OPERATION_PROFILES[index];
            GtkWidget *button;
            GtkWidget *box;
            GtkWidget *title;
            GtkWidget *detail;
            char detail_text[768];
            int enabled;

            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (commands == NULL ||
                !umi_command_registry_contains(
                    commands,
                    profile->command_id) ||
                !runtime_operation_matches_query(profile, query)) {
                continue;
            }

            button = gtk_button_new();
            box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            title = runtime_label(profile->title, 0.0F);

            (void)snprintf(
                detail_text,
                sizeof(detail_text),
                "%s — %s — %s",
                profile->category,
                profile->command_id,
                profile->description);
            detail = runtime_label(detail_text, 0.0F);

            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (button == NULL || box == NULL ||
                title == NULL || detail == NULL) {
                continue;
            }

            gtk_widget_add_css_class(
                button,
                "umicom-runtime-command-row");
            gtk_widget_add_css_class(
                detail,
                "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(box), title);
            gtk_box_append(GTK_BOX(box), detail);
            gtk_button_set_child(GTK_BUTTON(button), box);

            g_object_set_data_full(
                G_OBJECT(button),
                "umicom-operation-command",
                g_strdup(profile->command_id),
                g_free);
            g_signal_connect(
                button,
                "clicked",
                G_CALLBACK(runtime_on_operation_palette_result),
                runtime);

            /*
             * Commands with no argument can be evaluated immediately.
             * Argument-requiring commands become actionable when they are
             * registered; final enablement is re-evaluated after GTK collects
             * the actual path/text/thread/test argument.
             */
            enabled =
                profile->argument_kind ==
                    UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE
                ? umi_command_registry_is_enabled(
                    commands,
                    profile->command_id,
                    NULL)
                : 1;
            gtk_widget_set_sensitive(button, enabled);

            gtk_list_box_append(
                GTK_LIST_BOX(runtime->command_list),
                button);
        }
    }
}

/*
 * Provide the runtime on palette search changed operation used by this module and its
 * client applications.
 */
static void runtime_on_palette_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiWsCommandBarQuery query;

    /* The full palette and inline command bar share one prefix parser. The
     * search service receives plain text after the optional scope marker. */
    if (umi_gtk4_ws_command_bar_query(
            GTK_WIDGET(entry), &query) != UMI_STATUS_OK) {
        return;
    }
    runtime_command_palette_refresh(
        (UmiStudioGtkRuntimeChrome *)user_data,
        query.text);
}

/*
 * Provide the runtime on palette close operation used by this module and its client
 * applications.
 */
static gboolean runtime_on_palette_close(
    GtkWindow *window,
    gpointer user_data)
{
    (void)user_data;
    gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
    return TRUE;
}

/*
 * Provide the runtime build command palette operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_build_command_palette(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    GtkWidget *scroll;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->command_window = GTK_WIDGET(gtk_window_new());
    runtime->command_entry = umi_gtk4_ws_command_bar_create(
        "Search commands; use >, +, / or ? to narrow the scope");
    runtime->command_list = gtk_list_box_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_window == NULL ||
        runtime->command_entry == NULL ||
        runtime->command_list == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(runtime->command_window),
        "Search Commands and Menus");
    gtk_window_set_default_size(
        GTK_WINDOW(runtime->command_window),
        720,
        520);
    gtk_window_set_modal(GTK_WINDOW(runtime->command_window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(runtime->command_window),
        runtime->owner->window);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    scroll = runtime_scroll(runtime->command_list);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL || scroll == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);

    gtk_box_append(GTK_BOX(root), runtime->command_entry);
    gtk_box_append(GTK_BOX(root), scroll);
    gtk_window_set_child(GTK_WINDOW(runtime->command_window), root);

    g_signal_connect(
        runtime->command_entry,
        "search-changed",
        G_CALLBACK(runtime_on_palette_search_changed),
        runtime);
    g_signal_connect(
        runtime->command_window,
        "close-request",
        G_CALLBACK(runtime_on_palette_close),
        runtime);

    runtime_command_palette_refresh(runtime, "");
    gtk_widget_set_visible(runtime->command_window, FALSE);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime show command palette operation used by this module and its client
 * applications.
 */
static void runtime_show_command_palette(
    UmiStudioGtkRuntimeChrome *runtime)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->command_window == NULL) return;

    gtk_editable_set_text(GTK_EDITABLE(runtime->command_entry), "");
    runtime_command_palette_refresh(runtime, "");
    gtk_window_present(GTK_WINDOW(runtime->command_window));
    gtk_widget_grab_focus(runtime->command_entry);
}
