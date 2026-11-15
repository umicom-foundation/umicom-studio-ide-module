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

/* The former in-place list refresh and raw runtime callbacks are retained for
 * review. Detached result preparation and weak owner resolution below preserve
 * the command services while rejecting retired rows and observer reentry. */
#if 0


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

#endif

/* Build results away from the live scroll host. Weak runtime tokens and a
 * generation identify the current result set; retained rows cannot replay a
 * previous query after a refresh or after native owner removal. */
static UmiStatus RuntimeRefreshCommandPalette(UmiStudioGtkRuntimeChrome *runtime,
    const char *query);

static void RuntimePaletteRequestFree(gpointer data, GClosure *closure)
{
    (void)closure;
    runtime_file_dialog_request_free(data);
}

static void RuntimePaletteActivate(GtkButton *button, gpointer data, bool operation)
{
    UmiStudioFileDialogRequest *binding = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding, &window);
    if (runtime == NULL ||
        g_object_get_data(window, "umicom-command-selection-active") != NULL ||
        g_object_get_data(window, "umicom-command-refresh-active") != NULL ||
        g_object_get_data(window, "umicom-command-present-active") != NULL ||
        runtime->command_list == NULL || !gtk_widget_is_sensitive(GTK_WIDGET(button)) ||
        !gtk_widget_is_ancestor(GTK_WIDGET(button), runtime->command_list)) {
        g_clear_object(&window); return;
    }
    const char *borrowed = g_object_get_data(G_OBJECT(button),
        operation ? "umicom-operation-command" : "umicom-command-id");
    if (borrowed == NULL) { g_object_unref(window); return; }
    char *command = g_strdup(borrowed);
    const UmiStudioGtkOperationProfile *profile = operation ? runtime_operation_profile(command) : NULL;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { g_free(command); g_object_unref(window); return; }
    const uint64_t generation = runtime->command_generation;
    int enabled = 0;
    if (operation && profile != NULL) {
        UmiCommandRegistry *commands = runtime_registered_commands(runtime);
        enabled = commands != NULL && umi_command_registry_contains(commands, command) &&
            (profile->argument_kind != UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE ||
             umi_command_registry_is_enabled(commands, command, NULL));
    } else if (!operation) enabled = runtime_command_enabled(runtime, command);
    runtime = RuntimeResolveOwner(request, &window);
    if (!enabled || runtime == NULL || runtime->command_generation != generation) goto done;
    GObject *guard = g_object_ref(window);
    GtkWindow *palette = g_object_ref(GTK_WINDOW(runtime->command_window));
    g_object_set_data(guard, "umicom-command-selection-active", GINT_TO_POINTER(1));
    UmiStatus status = UmiGtk4HideOwnedWindow(GTK_WINDOW(window), palette);
    runtime = RuntimeResolveOwner(request, &window);
    if (status == UMI_STATUS_OK && runtime != NULL && runtime->command_generation == generation) {
        if (operation) (void)runtime_execute_operation_profile(runtime, profile);
        else (void)runtime_execute_command(runtime, command, NULL);
    }
    g_object_set_data(guard, "umicom-command-selection-active", NULL);
    g_object_unref(palette);
    g_object_unref(guard);
done:
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(command);
}

static void runtime_on_palette_result(GtkButton *button, gpointer data)
{
    RuntimePaletteActivate(button, data, false);
}
static void runtime_on_operation_palette_result(GtkButton *button, gpointer data)
{
    RuntimePaletteActivate(button, data, true);
}

static UmiStatus RuntimePaletteAppend(UmiStudioGtkRuntimeChrome *runtime,
    GtkListBox *list, const char *title_text, const char *detail_text,
    const char *command, bool operation, int enabled)
{
    UmiStudioFileDialogRequest *binding = runtime_file_dialog_request_new(runtime, 0U);
    if (binding == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *title = runtime_label(title_text, 0.0F);
    GtkWidget *detail = runtime_label(detail_text, 0.0F);
    gtk_widget_add_css_class(button, "umicom-runtime-command-row");
    gtk_widget_add_css_class(detail, "umicom-runtime-muted");
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), detail);
    gtk_button_set_child(GTK_BUTTON(button), box);
    g_object_set_data_full(G_OBJECT(button),
        operation ? "umicom-operation-command" : "umicom-command-id", g_strdup(command), g_free);
    g_signal_connect_data(button, "clicked", operation
        ? G_CALLBACK(runtime_on_operation_palette_result) : G_CALLBACK(runtime_on_palette_result),
        binding, RuntimePaletteRequestFree, 0);
    gtk_widget_set_sensitive(button, enabled);
    gtk_list_box_append(list, button);
    return UMI_STATUS_OK;
}

/* A failed preparation keeps the previous result set. Only one GTK child
 * replacement publishes the candidate. Resolve the runtime again afterwards:
 * a child observer can close Studio while widgets remain strongly referenced. */
static UmiStatus RuntimeRefreshCommandPalette(UmiStudioGtkRuntimeChrome *runtime,
    const char *query)
{
    if (runtime == NULL || runtime->runtime_bootstrap == NULL ||
        runtime->command_scroll == NULL) return UMI_STATUS_INVALID_STATE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    GtkWidget *candidate = NULL, *scroll = NULL;
    GObject *guard = NULL;
    char *owned_query = NULL;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(runtime->command_window))) goto done;
    if (g_object_get_data(window, "umicom-command-refresh-active") != NULL ||
        g_object_get_data(window, "umicom-command-selection-active") != NULL) {
        status = UMI_STATUS_BUSY; goto done;
    }
    if (runtime->command_generation == UINT64_MAX) { status = UMI_STATUS_CAPACITY_EXCEEDED; goto done; }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-command-refresh-active", GINT_TO_POINTER(1));
    owned_query = g_strdup(query != NULL ? query : "");
    UmiStudioRuntimeBindings *bindings = umi_studio_runtime_platform_bindings(runtime->runtime_bootstrap->platform);
    UmiStudioRuntimeCommandSearchResults results;
    if (bindings == NULL) { status = UMI_STATUS_UNAVAILABLE; goto done; }
    status = umi_studio_command_search(bindings, owned_query, &results);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status != UMI_STATUS_OK) goto done;
    candidate = g_object_ref_sink(gtk_list_box_new());
    for (size_t i = 0U; i < results.count; ++i) {
        const UmiStudioRuntimeCommandSearchResult *item = &results.items[i];
        /* Native admission is authoritative; a generic enabled alias must not
         * bypass the native document, build or lifetime preconditions. */
        int enabled = runtime_command_enabled(runtime, item->command_id);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
        status = RuntimePaletteAppend(runtime, GTK_LIST_BOX(candidate), item->title,
            item->command_id, item->command_id, false, enabled);
        if (status != UMI_STATUS_OK) goto done;
    }
    for (size_t i = 0U; i < runtime_operation_profile_count(); ++i) {
        const UmiStudioGtkOperationProfile *profile = &RUNTIME_OPERATION_PROFILES[i];
        UmiCommandRegistry *commands = runtime_registered_commands(runtime);
        if (commands == NULL || !umi_command_registry_contains(commands, profile->command_id) ||
            !runtime_operation_matches_query(profile, owned_query)) continue;
        int enabled = profile->argument_kind == UMI_STUDIO_GTK_OPERATION_ARGUMENT_NONE
            ? umi_command_registry_is_enabled(commands, profile->command_id, NULL) : 1;
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
        char detail[768];
        (void)snprintf(detail, sizeof detail, "%s — %s — %s", profile->category,
            profile->command_id, profile->description);
        status = RuntimePaletteAppend(runtime, GTK_LIST_BOX(candidate), profile->title,
            detail, profile->command_id, true, enabled);
        if (status != UMI_STATUS_OK) goto done;
    }
    scroll = g_object_ref(runtime->command_scroll);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), candidate);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    /* GTK may insert a viewport around a non-scrollable result list. Check
     * ancestry rather than assuming the list is the scroll host’s direct child. */
    if (!gtk_widget_is_ancestor(candidate, scroll)) {
        runtime->command_list = NULL;
        status = UMI_STATUS_CANCELLED; goto done;
    }
    runtime->command_list = candidate;
    ++runtime->command_generation;
    status = UMI_STATUS_OK;
done:
    /* A failed query must not leave earlier results actionable under new text.
     * Keep their content for diagnosis, but only a successful refresh re-enables
     * a complete candidate. Observers may close the owner during this update. */
    if (status != UMI_STATUS_OK && guard != NULL) {
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime != NULL && runtime->command_list != NULL)
            gtk_widget_set_sensitive(runtime->command_list, FALSE);
    }
    if (guard != NULL) g_object_set_data(guard, "umicom-command-refresh-active", NULL);
    g_clear_object(&guard); g_clear_object(&scroll); g_clear_object(&candidate);
    g_clear_object(&window); g_free(owned_query);
    runtime_file_dialog_request_free(request);
    return status;
}

static void runtime_on_palette_search_changed(GtkSearchEntry *entry, gpointer data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    UmiWsCommandBarQuery query;
    if (runtime != NULL && runtime->command_entry == GTK_WIDGET(entry) &&
        umi_gtk4_ws_command_bar_query(GTK_WIDGET(entry), &query) == UMI_STATUS_OK)
        (void)RuntimeRefreshCommandPalette(runtime, query.text);
    g_clear_object(&window);
}

static gboolean runtime_on_palette_close(GtkWindow *window, gpointer data)
{
    (void)data;
    gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
    return TRUE;
}

static UmiStatus runtime_build_command_palette(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    runtime->command_window = GTK_WIDGET(gtk_window_new());
    runtime->command_entry = umi_gtk4_ws_command_bar_create(
        "Search commands; use >, +, / or ? to narrow the scope");
    if (runtime->command_entry == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    runtime->command_list = gtk_list_box_new();
    gtk_window_set_title(GTK_WINDOW(runtime->command_window), "Search Commands and Menus");
    gtk_window_set_default_size(GTK_WINDOW(runtime->command_window), 720, 520);
    gtk_window_set_modal(GTK_WINDOW(runtime->command_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(runtime->command_window), runtime->owner->window);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    runtime->command_scroll = runtime_scroll(runtime->command_list);
    if (runtime->command_scroll == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    gtk_widget_set_margin_top(root, 12); gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12); gtk_widget_set_margin_end(root, 12);
    gtk_box_append(GTK_BOX(root), runtime->command_entry);
    gtk_box_append(GTK_BOX(root), runtime->command_scroll);
    gtk_window_set_child(GTK_WINDOW(runtime->command_window), root);
    UmiStudioFileDialogRequest *binding = runtime_file_dialog_request_new(runtime, 0U);
    if (binding == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    g_signal_connect_data(runtime->command_entry, "search-changed",
        G_CALLBACK(runtime_on_palette_search_changed), binding, RuntimePaletteRequestFree, 0);
    g_signal_connect(runtime->command_window, "close-request", G_CALLBACK(runtime_on_palette_close), NULL);
    return RuntimeRefreshCommandPalette(runtime, "");
}

/* Menu, toolbar and keyboard share presentation and its actual result. Reset
 * query observers and focus observers may both retire the application owner. */
static UmiStatus RuntimePresentCommandPalette(UmiStudioGtkRuntimeChrome *runtime, const char *argument)
{
    if (argument != NULL && argument[0] != '\0') return UMI_STATUS_INVALID_ARGUMENT;
    if (runtime == NULL || runtime->command_window == NULL || runtime->command_entry == NULL)
        return UMI_STATUS_UNAVAILABLE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    GObject *guard = NULL;
    GtkWidget *entry = NULL;
    GtkWindow *palette = NULL;
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(runtime->command_window))) goto done;
    if (g_object_get_data(window, "umicom-command-present-active") != NULL ||
        g_object_get_data(window, "umicom-command-selection-active") != NULL ||
        g_object_get_data(window, "umicom-command-refresh-active") != NULL) {
        status = UMI_STATUS_BUSY; goto done;
    }
    guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-command-present-active", GINT_TO_POINTER(1));
    entry = g_object_ref(runtime->command_entry);
    palette = g_object_ref(GTK_WINDOW(runtime->command_window));
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) goto done;
    status = RuntimeRefreshCommandPalette(runtime, "");
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status != UMI_STATUS_OK) goto done;
    status = UmiGtk4PresentOwnedWindow(GTK_WINDOW(window), palette);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime == NULL) { status = UMI_STATUS_CANCELLED; goto done; }
    if (status == UMI_STATUS_OK) {
        (void)gtk_widget_grab_focus(entry);
        runtime = RuntimeResolveOwner(request, &window);
        if (runtime == NULL || !UmiGtk4WindowIsOpen(palette) ||
            !gtk_widget_get_visible(GTK_WIDGET(palette)) ||
            gtk_window_get_transient_for(palette) != GTK_WINDOW(window)) status = UMI_STATUS_CANCELLED;
    }
done:
    if (guard != NULL) g_object_set_data(guard, "umicom-command-present-active", NULL);
    g_clear_object(&guard); g_clear_object(&window);
    g_clear_object(&entry); g_clear_object(&palette);
    runtime_file_dialog_request_free(request);
    return status;
}

static void runtime_show_command_palette(UmiStudioGtkRuntimeChrome *runtime)
{
    (void)RuntimePresentCommandPalette(runtime, NULL);
}
