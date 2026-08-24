/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_command_palette.inc
 *
 * PURPOSE:
 *   Render Framework command/menu search as a GTK4 command palette.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_command_palette_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *query);

static void runtime_on_palette_result(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-id");

    if (runtime == NULL || command_id == NULL) return;

    gtk_widget_set_visible(runtime->command_window, FALSE);
    (void)runtime_execute_command(runtime, command_id, NULL);
}

static void runtime_command_palette_refresh(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *query)
{
    UmiStudioRuntimeBindings *bindings;
    UmiStudioRuntimeCommandSearchResults results;
    size_t index;

    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        runtime->command_list == NULL) {
        return;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    if (bindings == NULL) return;

    runtime_clear_list_box(runtime->command_list);

    if (umi_studio_command_search(
            bindings,
            query != NULL ? query : "",
            &results) != UMI_STATUS_OK) {
        return;
    }

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
        gtk_widget_set_sensitive(button, item->enabled);

        gtk_list_box_append(
            GTK_LIST_BOX(runtime->command_list),
            button);
    }
}

static void runtime_on_palette_search_changed(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    runtime_command_palette_refresh(
        (UmiStudioGtkRuntimeChrome *)user_data,
        gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static gboolean runtime_on_palette_close(
    GtkWindow *window,
    gpointer user_data)
{
    (void)user_data;
    gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
    return TRUE;
}

static UmiStatus runtime_build_command_palette(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    GtkWidget *scroll;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->command_window = GTK_WIDGET(gtk_window_new());
    runtime->command_entry = gtk_search_entry_new();
    runtime->command_list = gtk_list_box_new();

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

static void runtime_show_command_palette(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->command_window == NULL) return;

    gtk_editable_set_text(GTK_EDITABLE(runtime->command_entry), "");
    runtime_command_palette_refresh(runtime, "");
    gtk_window_present(GTK_WINDOW(runtime->command_window));
    gtk_widget_grab_focus(runtime->command_entry);
}
