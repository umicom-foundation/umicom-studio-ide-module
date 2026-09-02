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

    status = runtime_prepare_session_path(runtime);
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
    options.session_path = runtime->session_path;
    options.session_prefix = "studio.gtk.runtime";
    options.restore_session = 1;

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

/*
 * Provide the runtime destroy framework operation used by this module and its client
 * applications.
 */
static void runtime_destroy_framework(
    UmiStudioGtkRuntimeChrome *runtime)
{
    int kind;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime->refresh_source != 0U) {
        g_source_remove(runtime->refresh_source);
        runtime->refresh_source = 0U;
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

    /* The GTK tree owns the visible widgets. These calls release the Framework
     * controllers that track command and identity state. */
    (void)umi_gtk4_ws_command_bar_set_activated_handler(
        runtime->command_bar, NULL, NULL);
    umi_gtk4_ws_command_bar_destroy(runtime->command_bar);
    runtime->command_bar = NULL;
    umi_gtk4_ws_shell_header_destroy(runtime->brand_identity);
    runtime->brand_identity = NULL;

    /* Detached panels are top-level GTK windows and therefore must be closed
     * explicitly before the runtime state they reference is released. */
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
        ++kind) {
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (runtime->surface_windows[kind] != NULL) {
            g_signal_handlers_disconnect_by_func(
                runtime->surface_windows[kind],
                G_CALLBACK(runtime_on_floating_surface_close),
                runtime);
            gtk_window_destroy(GTK_WINDOW(runtime->surface_windows[kind]));
            runtime->surface_windows[kind] = NULL;
            runtime->surface_floating[kind] = 0;
        }
    }

    umi_studio_runtime_bootstrap_destroy(
        runtime->runtime_bootstrap);
    runtime->runtime_bootstrap = NULL;

    umi_ide_integration_platform_destroy(
        runtime->ide_platform);
    runtime->ide_platform = NULL;

    umi_application_shell_registry_destroy(
        runtime->shell_registry);
    runtime->shell_registry = NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->css_provider != NULL) {
        g_object_unref(runtime->css_provider);
        runtime->css_provider = NULL;
    }
}
