/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_framework.inc
 *
 * PURPOSE:
 *   Construct the Framework Studio Runtime over the already-authoritative Studio
 *   UI/services. No duplicate document, test or language service is created.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_prepare_session_path(
    UmiStudioGtkRuntimeChrome *runtime)
{
    const char *config_root;
    char *directory;
    char *path;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    config_root = g_get_user_config_dir();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (config_root == NULL) return UMI_STATUS_UNAVAILABLE;

    directory = g_build_filename(
        config_root,
        "umicom",
        "studio",
        NULL);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (directory == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    /* Apply this branch only when its contract condition is satisfied. */
    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_free(directory);
        return UMI_STATUS_IO_ERROR;
    }

    path = g_build_filename(
        directory,
        "gtk-runtime-session.state",
        NULL);
    g_free(directory);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (path == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (strlen(path) >= sizeof(runtime->session_path)) {
        g_free(path);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)strcpy(runtime->session_path, path);
    g_free(path);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime prepare workspace identity operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_prepare_workspace_identity(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char *root;
    char *name;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    root = g_get_current_dir();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    name = g_path_get_basename(root);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (name == NULL) {
        g_free(root);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (strlen(root) >= sizeof(runtime->workspace_root) ||
        strlen(name) >= sizeof(runtime->workspace_name)) {
        g_free(name);
        g_free(root);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)strcpy(runtime->workspace_root, root);
    (void)strcpy(runtime->workspace_name, name);
    g_free(name);
    g_free(root);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime create framework operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_create_framework(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkbench *ui_workbench;
    UmiCommandRegistry *commands;
    UmiStudioServices *services;
    UmiStudioTestService *tests;
    UmiStudioLanguageService *language;
    UmiIdeIntegrationBindings ide_bindings;
    UmiStudioRuntimeBindings runtime_bindings;
    UmiStudioRuntimeBootstrapOptions options;
    const UmiApplicationShellPersonaDefinition *persona;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = runtime_prepare_workspace_identity(runtime);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Isolated native tests and embedding callers may disable all session
     * storage; do not even create the user configuration directory then. */
    status = runtime->owner->options.persist_session
        ? runtime_prepare_session_path(runtime) : UMI_STATUS_OK;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    ui_workbench = umi_studio_ui_workbench(runtime->owner->ui);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (ui_workbench == NULL) return UMI_STATUS_INVALID_STATE;

    commands = umi_ui_workbench_commands(ui_workbench);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands == NULL) return UMI_STATUS_INVALID_STATE;

    status = umi_application_shell_registry_create(
        &runtime->shell_registry);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* The source-preview service is Framework-owned and provider-neutral. Its
     * safe built-ins are available before GTK creates the Live Preview panel. */
    status = umi_browser_source_preview_service_create(
        &runtime->source_preview_service);
    /* Built-ins can only be installed into a successfully allocated service. */
    if (status == UMI_STATUS_OK) {
        status = umi_browser_source_preview_service_install_builtins(
            runtime->source_preview_service);
    }
    /* Attachment stops cleanly when the preview catalogue cannot be prepared. */
    if (status != UMI_STATUS_OK) return status;

    umi_application_shell_state_init(&runtime->shell_state);
    umi_application_shell_layout_init(
        &runtime->shell_layout,
        "umicom.studio.gtk.runtime",
        "Umicom Studio GTK4 Runtime");

    persona = umi_application_shell_persona_find(
        "umicom.shell.persona.studio-ide");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (persona == NULL) return UMI_STATUS_NOT_FOUND;

    status = umi_application_shell_persona_install(
        runtime->shell_registry,
        persona);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    umi_ide_integration_bindings_init(&ide_bindings);

    status = umi_ide_integration_bind_documents(
        &ide_bindings,
        umi_studio_ui_documents(runtime->owner->ui));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    services = umi_studio_ui_services(runtime->owner->ui);
    tests = services != NULL
        ? umi_studio_services_tests(services)
        : NULL;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (tests != NULL) {
        status = umi_ide_integration_bind_tests(
            &ide_bindings,
            umi_studio_test_service_platform(tests));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    language = services != NULL
        ? umi_studio_services_language(services)
        : NULL;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (language != NULL) {
        status = umi_ide_integration_bind_language(
            &ide_bindings,
            umi_studio_language_service_model(language));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    status = umi_ide_integration_platform_create(
        runtime->workspace_root,
        &ide_bindings,
        &runtime->ide_platform);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /*
     * The current Studio application still exposes Source Control, Debug and AI
     * through older application facades whose types intentionally differ from
     * the newer IDE Integration bindings. Use the interactive-development gate
     * profile so those temporarily unbound facades do not make the visible GTK
     * workbench appear permanently blocked. Their authoritative snapshots are
     * still rendered below and later phases can bind them as facades converge.
     */
    {
        UmiIdeWorkflowPolicy policy;
        status = umi_ide_workflow_profile_development(&policy);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_ide_integration_platform_set_workflow_policy(
                runtime->ide_platform,
                &policy);
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    umi_studio_runtime_bindings_init(&runtime_bindings);

    status = umi_studio_runtime_bind_shell(
        &runtime_bindings,
        runtime->shell_registry,
        &runtime->shell_state,
        &runtime->shell_layout);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_commands(
            &runtime_bindings,
            commands);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_ide(
            &runtime_bindings,
            runtime->ide_platform);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_documents(
            &runtime_bindings,
            umi_studio_ui_documents(runtime->owner->ui));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    (void)memset(&options, 0, sizeof(options));
    options.workspace_name = runtime->workspace_name;
    options.actor_id = "umicom-studio-user";
    options.session_path = runtime->owner->options.persist_session
        ? runtime->session_path : NULL;
    options.session_prefix = "studio.gtk.runtime";
    options.restore_session = runtime->owner->options.persist_session &&
        runtime->owner->options.restore_session;

    status = umi_studio_runtime_bootstrap_create(
        &runtime_bindings,
        &options,
        &runtime->runtime_bootstrap);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        runtime->runtime_ready = 1;
    }

    return status;
}

/* Remove callbacks with one borrowed data pointer from widgets, text buffers
 * and controllers, including objects held by callers after their old frame dies. */
static void runtime_disconnect_signal_data_tree(GtkWidget *root, gpointer data)
{
    GtkWidget *child;
    GListModel *controllers;
    guint index;
    if (root == NULL || data == NULL) return;
    g_signal_handlers_disconnect_by_data(root, data);
    if (GTK_IS_TEXT_VIEW(root))
        g_signal_handlers_disconnect_by_data(gtk_text_view_get_buffer(GTK_TEXT_VIEW(root)), data);
    controllers = gtk_widget_observe_controllers(root);
    for (index = 0U; index < g_list_model_get_n_items(controllers); ++index) {
        GObject *controller = g_list_model_get_item(controllers, index);
        g_signal_handlers_disconnect_by_data(controller, data);
        g_object_unref(controller);
    }
    g_object_unref(controllers);
    for (child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child))
        runtime_disconnect_signal_data_tree(child, data);
}

/* Nested designer records borrow services too. Cancel their work before either
 * a deliberate provider refresh or final host teardown releases their bodies. */
static void runtime_invalidate_surface_nodes(GtkWidget *widget, GtkWidget *scope)
{
    UmiStudioGtkDesignerPaletteState *palette;
    UmiStudioGtkDesignerPropertyState *property;
    UmiStudioGtkDesignerCanvasState *canvas;
    UmiStudioGtkDesignerSourceState *source;
    GtkWidget *child;
    palette = g_object_get_data(G_OBJECT(widget), "umicom-designer-palette-state");
    property = g_object_get_data(G_OBJECT(widget), "umicom-designer-property-state");
    canvas = g_object_get_data(G_OBJECT(widget), "umicom-designer-canvas-state");
    source = g_object_get_data(G_OBJECT(widget), "umicom-designer-source-state");
    if (palette != NULL) {
        palette->runtime = NULL;
        runtime_disconnect_signal_data_tree(scope, palette);
    }
    if (property != NULL) {
        property->runtime = NULL;
        runtime_disconnect_signal_data_tree(scope, property);
    }
    if (canvas != NULL) {
        if (canvas->interaction.active)
            umi_studio_designer_cancel_surface_interaction(&canvas->interaction);
        canvas->runtime = NULL;
        canvas->designer = NULL;
        runtime_disconnect_signal_data_tree(scope, canvas);
    }
    if (source != NULL) {
        if (source->debounce_source != 0U) {
            g_source_remove(source->debounce_source);
            source->debounce_source = 0U;
        }
        source->runtime = NULL;
        runtime_disconnect_signal_data_tree(scope, source);
    }
    for (child = gtk_widget_get_first_child(widget); child != NULL;
         child = gtk_widget_get_next_sibling(child))
        runtime_invalidate_surface_nodes(child, scope);
}

/* Disconnect view callbacks while their real retained bodies are still alive. */
static void runtime_invalidate_surface_tree(GtkWidget *root, UmiStudioGtkRuntimeChrome *runtime)
{
    if (root == NULL) return;
    runtime_disconnect_signal_data_tree(root, runtime);
    runtime_invalidate_surface_nodes(root, root);
}

/* Close only this runtime's text prompts and invalidate retained prompt
 * controls before their borrowed host or accepted callback becomes invalid. */
static void runtime_destroy_text_prompts(UmiStudioGtkRuntimeChrome *runtime)
{
    GListModel *windows = gtk_window_get_toplevels();
    GPtrArray *owned = g_ptr_array_new_with_free_func(g_object_unref);
    guint index;
    for (index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *window = g_list_model_get_item(windows, index);
        UmiStudioGtkPromptContext *prompt =
            g_object_get_data(G_OBJECT(window), "umicom-prompt-context");
        if (prompt != NULL && prompt->runtime == runtime) g_ptr_array_add(owned, window);
        else g_object_unref(window);
    }
    for (index = 0U; index < owned->len; ++index) {
        GtkWidget *window = g_ptr_array_index(owned, index);
        UmiStudioGtkPromptContext *prompt =
            g_object_get_data(G_OBJECT(window), "umicom-prompt-context");
        prompt->runtime = NULL;
        prompt->accepted = NULL;
        runtime_disconnect_signal_data_tree(window, prompt);
        gtk_window_destroy(GTK_WINDOW(window));
    }
    g_ptr_array_unref(owned);
}

/* Release callbacks and retained GTK bodies before their Framework services. */
static void runtime_destroy_framework(
    UmiStudioGtkRuntimeChrome *runtime)
{
    int kind;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    UmiGtk4DeveloperDialogDestroy(runtime->project_dialog);
    runtime->project_dialog = NULL;
    UmiGtk4DeveloperDialogDestroy(runtime->build_settings_dialog);
    runtime->build_settings_dialog = NULL;
    runtime_destroy_text_prompts(runtime);
    /* Stop runtime-owned callbacks before the host releases retained bodies. */
    if (runtime->owner != NULL && runtime->owner->window != NULL)
        runtime_disconnect_signal_data_tree(GTK_WIDGET(runtime->owner->window), runtime);
    runtime_invalidate_surface_tree(runtime->outer_root, runtime);
    runtime_invalidate_surface_tree(runtime->centre_stack, runtime);
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST; ++kind)
        runtime_invalidate_surface_tree(runtime->surface_panels[kind], runtime);
    runtime_invalidate_surface_tree(runtime->command_window, runtime);
    runtime_invalidate_surface_tree(runtime->layout_window, runtime);
    runtime_invalidate_surface_tree(runtime->window_catalogue_window, runtime);
    if (runtime->notification_source != 0U) {
        g_source_remove(runtime->notification_source);
        runtime->notification_source = 0U;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime->refresh_source != 0U) {
        g_source_remove(runtime->refresh_source);
        runtime->refresh_source = 0U;
    }
    /* A pending designer refresh also borrows the runtime pointer, so cancel it
     * before the workbench state is released. */
    if (runtime->designer_refresh_source != 0U) {
        g_source_remove(runtime->designer_refresh_source);
        runtime->designer_refresh_source = 0U;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->command_window));
        runtime->command_window = NULL;
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->layout_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->layout_window));
        runtime->layout_window = NULL;
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->window_catalogue_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->window_catalogue_window));
        runtime->window_catalogue_window = NULL;
    }

    /* The GTK tree owns the visible widgets. Release the command controller
     * here; the workbench owns the separate topmost titlebar controller. */
    (void)umi_gtk4_ws_command_bar_set_activated_handler(
        runtime->command_bar, NULL, NULL);
    umi_gtk4_ws_command_bar_destroy(runtime->command_bar);
    runtime->command_bar = NULL;

    /* Cancel deferred library requests before their borrowed workspace is released. */
    umi_gtk4_ws_layout_library_destroy(runtime->workspace_library);
    runtime->workspace_library = NULL;

    /* The shared host cancels gestures and owns every detached native window.
     * Restore the original adapter tree before its established teardown runs. */
    umi_gtk4_workspace_layout_host_destroy(runtime->workspace_host);
    runtime->workspace_host = NULL;
    if (runtime->editor_adapter_root != NULL) {
        GtkWidget *parent = gtk_widget_get_parent(runtime->editor_adapter_root);
        if (parent != NULL && GTK_IS_BOX(parent))
            gtk_box_remove(GTK_BOX(parent), runtime->editor_adapter_root);
        if (runtime->owner != NULL && runtime->owner->window != NULL)
            gtk_window_set_child(runtime->owner->window, runtime->editor_adapter_root);
        g_clear_object(&runtime->editor_adapter_root);
    }
    g_clear_object(&runtime->centre_stack);
    g_clear_object(&runtime->outer_root);
    umi_studio_professional_workspace_destroy(runtime->professional_workspace);
    runtime->professional_workspace = NULL;
    /* Detach the build-profile borrower before the native owner releases its
     * Data Server. Service-owned and test connections keep their original owner. */
    if (runtime->owner != NULL && runtime->owner->ui != NULL)
        UmiStudioBuildProfilesDetach(umi_studio_ui_services(runtime->owner->ui));
    /* Borrowed test or service connections remain with their original owner. */
    if (runtime->owns_workspace_storage)
        umi_data_server_destroy(runtime->workspace_storage);
    runtime->workspace_storage = NULL;
    runtime->owns_workspace_storage = 0;

    umi_studio_runtime_bootstrap_destroy(
        runtime->runtime_bootstrap);
    runtime->runtime_bootstrap = NULL;

    umi_ide_integration_platform_destroy(
        runtime->ide_platform);
    runtime->ide_platform = NULL;

    umi_application_shell_registry_destroy(
        runtime->shell_registry);
    runtime->shell_registry = NULL;

    /* Provider contexts are caller-owned, while the copied provider catalogue
     * itself belongs to the Framework service created during attachment. */
    umi_browser_source_preview_service_destroy(
        runtime->source_preview_service);
    runtime->source_preview_service = NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->css_provider != NULL) {
        g_object_unref(runtime->css_provider);
        runtime->css_provider = NULL;
    }
}
