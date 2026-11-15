/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_layout_browser.inc
 *
 * PURPOSE:
 *   Present and switch the seven Framework semantic Studio layouts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* The former raw runtime callback could outlive its owner or reuse it after
 * a hide observer. The weak binding replaces it; retain the old code for review. */
#if 0
static void runtime_on_layout_button(GtkButton *button, gpointer user_data)
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

    gtk_widget_set_visible(runtime->layout_window, FALSE);
    (void)runtime_execute_command(runtime, command_id, NULL);
}

#endif

typedef struct RuntimeLayoutChoice {
    UmiStudioFileDialogRequest *owner;
    GWeakRef browser;
} RuntimeLayoutChoice;

static void RuntimeLayoutChoiceFree(gpointer data, GClosure *closure)
{
    (void)closure;
    RuntimeLayoutChoice *choice = data;
    runtime_file_dialog_request_free(choice->owner);
    g_weak_ref_clear(&choice->browser);
    g_free(choice);
}

/* A retained row must not retain its workbench. Copy everything needed before
 * hiding the browser: a visibility observer may release the owner and rows. */
static void runtime_on_layout_button(GtkButton *button, gpointer data)
{
    RuntimeLayoutChoice *choice = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(choice->owner, &window);
    if (runtime == NULL) { g_clear_object(&window); return; }
    const char *borrowed = g_object_get_data(G_OBJECT(button), "umicom-command-id");
    if (borrowed == NULL || g_object_get_data(window, "umicom-layout-selection-active") != NULL) {
        g_clear_object(&window);
        return;
    }
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    if (model == NULL || model->edit_active) { g_clear_object(&window); return; }
    GObject *browser = g_weak_ref_get(&choice->browser);
    if (browser == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(browser))) {
        g_clear_object(&browser); g_clear_object(&window); return;
    }
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) { g_object_unref(browser); g_object_unref(window); return; }
    char *command = g_strdup(borrowed);
    GObject *guard = g_object_ref(window);
    g_object_set_data(guard, "umicom-layout-selection-active", GINT_TO_POINTER(1));
    gtk_widget_set_visible(GTK_WIDGET(browser), FALSE);
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) (void)runtime_execute_command(runtime, command, NULL);
    g_object_set_data(guard, "umicom-layout-selection-active", NULL);
    g_object_unref(guard);
    g_clear_object(&window);
    g_object_unref(browser);
    runtime_file_dialog_request_free(request);
    g_free(command);
}

/*
 * Provide the runtime build layout browser operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_build_layout_browser(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    runtime->layout_window = GTK_WIDGET(gtk_window_new());
    runtime->layout_list = gtk_list_box_new();
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->layout_window == NULL ||
        runtime->layout_list == NULL ||
        root == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(runtime->layout_window),
        "Studio Layout Browser");
    gtk_window_set_default_size(
        GTK_WINDOW(runtime->layout_window), 560, 480);
    gtk_window_set_modal(GTK_WINDOW(runtime->layout_window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(runtime->layout_window),
        runtime->owner->window);

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);

    gtk_box_append(
        GTK_BOX(root),
        runtime_section_title("Framework semantic layouts"));

    /* Attach the partial body before creating weak row bindings. If a binding
     * allocation fails, normal owner teardown also releases these widgets. */
    gtk_box_append(GTK_BOX(root), runtime_scroll(runtime->layout_list));
    gtk_window_set_child(GTK_WINDOW(runtime->layout_window), root);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_LAYOUT_PROFILES) /
                 sizeof(RUNTIME_LAYOUT_PROFILES[0]);
         ++index) {
        const UmiStudioGtkLayoutProfile *profile =
            &RUNTIME_LAYOUT_PROFILES[index];
        GtkWidget *button = gtk_button_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        GtkWidget *title = runtime_label(profile->label, 0.0F);
        GtkWidget *detail = runtime_label(profile->description, 0.0F);

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button == NULL || box == NULL ||
            title == NULL || detail == NULL) {
            continue;
        }

        gtk_widget_add_css_class(detail, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(box), title);
        gtk_box_append(GTK_BOX(box), detail);
        gtk_button_set_child(GTK_BUTTON(button), box);

        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(profile->command_id),
            g_free);
        /* Weak owner and browser identity replace the borrowed runtime. */
#if 0
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_layout_button),
            runtime);
#endif
        RuntimeLayoutChoice *choice = g_try_new0(RuntimeLayoutChoice, 1);
        if (choice == NULL) {
            g_object_ref_sink(button); g_object_unref(button);
            return UMI_STATUS_OUT_OF_MEMORY;
        }
        choice->owner = runtime_file_dialog_request_new(runtime, 0U);
        if (choice->owner == NULL) {
            g_free(choice); g_object_ref_sink(button); g_object_unref(button);
            return UMI_STATUS_OUT_OF_MEMORY;
        }
        g_weak_ref_init(&choice->browser, G_OBJECT(runtime->layout_window));
        g_signal_connect_data(button, "clicked", G_CALLBACK(runtime_on_layout_button),
            choice, RuntimeLayoutChoiceFree, 0);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, profile->command_id));

        gtk_list_box_append(
            GTK_LIST_BOX(runtime->layout_list), button);
    }

    /* Earlier attachment above covers partial construction. Retain the former
     * late ownership transfer for review; reparenting the same body is unnecessary. */
#if 0
    gtk_box_append(GTK_BOX(root), runtime_scroll(runtime->layout_list));
    gtk_window_set_child(GTK_WINDOW(runtime->layout_window), root);
#endif

    g_signal_connect(
        runtime->layout_window,
        "close-request",
        G_CALLBACK(runtime_on_palette_close),
        runtime);

    gtk_widget_set_visible(runtime->layout_window, FALSE);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime show layout browser operation used by this module and its client
 * applications.
 */
/* The unchecked presentation is retained for review. Shared presentation
 * reports cancellation and reentry; Studio also rechecks its runtime token. */
#if 0
static void runtime_show_layout_browser(
    UmiStudioGtkRuntimeChrome *runtime)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime != NULL && runtime->layout_window != NULL) {
        gtk_window_present(GTK_WINDOW(runtime->layout_window));
    }
}
#endif

/* Keep the layout model untouched until the user chooses a row. The shared
 * presenter checks GTK ownership; the existing Studio token additionally
 * detects a detached runtime while its native window is still retained. */
static UmiStatus RuntimePresentLayoutBrowser(
    UmiStudioGtkRuntimeChrome *runtime, const char *argument)
{
    if (runtime == NULL || runtime->owner == NULL ||
        (argument != NULL && argument[0] != '\0'))
        return UMI_STATUS_INVALID_ARGUMENT;
    if (g_object_get_data(G_OBJECT(runtime->owner->window),
            "umicom-layout-selection-active") != NULL)
        return UMI_STATUS_BUSY;
    if (!GTK_IS_WINDOW(runtime->layout_window)) return UMI_STATUS_UNAVAILABLE;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    UmiStatus status = UmiGtk4PresentOwnedWindow(runtime->owner->window,
        GTK_WINDOW(runtime->layout_window));
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    return status;
}

/* Preserve keyboard and toolbar callers while the status-returning route is
 * used by commands. Presentation never saves or restores a workspace. */
static void runtime_show_layout_browser(UmiStudioGtkRuntimeChrome *runtime)
{
    (void)RuntimePresentLayoutBrowser(runtime, NULL);
}
