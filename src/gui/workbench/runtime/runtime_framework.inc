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

    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    config_root = g_get_user_config_dir();
    if (config_root == NULL) return UMI_STATUS_UNAVAILABLE;

    directory = g_build_filename(
        config_root,
        "umicom",
        "studio",
        NULL);
    if (directory == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_free(directory);
        return UMI_STATUS_IO_ERROR;
    }

    path = g_build_filename(
        directory,
        "gtk-runtime-session.state",
        NULL);
    g_free(directory);

    if (path == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    if (strlen(path) >= sizeof(runtime->session_path)) {
        g_free(path);
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    (void)strcpy(runtime->session_path, path);
    g_free(path);
    return UMI_STATUS_OK;
}

static UmiStatus runtime_prepare_workspace_identity(
    UmiStudioGtkRuntimeChrome *runtime)
{
    char *root;
    char *name;

    if (runtime == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    root = g_get_current_dir();
    if (root == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    name = g_path_get_basename(root);
    if (name == NULL) {
        g_free(root);
        return UMI_STATUS_OUT_OF_MEMORY;
    }

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

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = runtime_prepare_workspace_identity(runtime);
    if (status != UMI_STATUS_OK) return status;

    status = runtime_prepare_session_path(runtime);
    if (status != UMI_STATUS_OK) return status;

    ui_workbench = umi_studio_ui_workbench(runtime->owner->ui);
    if (ui_workbench == NULL) return UMI_STATUS_INVALID_STATE;

    commands = umi_ui_workbench_commands(ui_workbench);
    if (commands == NULL) return UMI_STATUS_INVALID_STATE;

    status = umi_application_shell_registry_create(
        &runtime->shell_registry);
    if (status != UMI_STATUS_OK) return status;

    umi_application_shell_state_init(&runtime->shell_state);
    umi_application_shell_layout_init(
        &runtime->shell_layout,
        "umicom.studio.gtk.runtime",
        "Umicom Studio GTK4 Runtime");

    persona = umi_application_shell_persona_find(
        "umicom.shell.persona.studio-ide");
    if (persona == NULL) return UMI_STATUS_NOT_FOUND;

    status = umi_application_shell_persona_install(
        runtime->shell_registry,
        persona);
    if (status != UMI_STATUS_OK) return status;

    umi_ide_integration_bindings_init(&ide_bindings);

    status = umi_ide_integration_bind_documents(
        &ide_bindings,
        umi_studio_ui_documents(runtime->owner->ui));
    if (status != UMI_STATUS_OK) return status;

    services = umi_studio_ui_services(runtime->owner->ui);
    tests = services != NULL
        ? umi_studio_services_tests(services)
        : NULL;
    if (tests != NULL) {
        status = umi_ide_integration_bind_tests(
            &ide_bindings,
            umi_studio_test_service_platform(tests));
        if (status != UMI_STATUS_OK) return status;
    }

    language = services != NULL
        ? umi_studio_services_language(services)
        : NULL;
    if (language != NULL) {
        status = umi_ide_integration_bind_language(
            &ide_bindings,
            umi_studio_language_service_model(language));
        if (status != UMI_STATUS_OK) return status;
    }

    status = umi_ide_integration_platform_create(
        runtime->workspace_root,
        &ide_bindings,
        &runtime->ide_platform);
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
        if (status == UMI_STATUS_OK) {
            status = umi_ide_integration_platform_set_workflow_policy(
                runtime->ide_platform,
                &policy);
        }
        if (status != UMI_STATUS_OK) return status;
    }

    umi_studio_runtime_bindings_init(&runtime_bindings);

    status = umi_studio_runtime_bind_shell(
        &runtime_bindings,
        runtime->shell_registry,
        &runtime->shell_state,
        &runtime->shell_layout);
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_commands(
            &runtime_bindings,
            commands);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_ide(
            &runtime_bindings,
            runtime->ide_platform);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_runtime_bind_documents(
            &runtime_bindings,
            umi_studio_ui_documents(runtime->owner->ui));
    }
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

    if (status == UMI_STATUS_OK) {
        runtime->runtime_ready = 1;
    }

    return status;
}

static void runtime_destroy_framework(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL) return;

    if (runtime->refresh_source != 0U) {
        g_source_remove(runtime->refresh_source);
        runtime->refresh_source = 0U;
    }

    if (runtime->command_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->command_window));
        runtime->command_window = NULL;
    }
    if (runtime->layout_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->layout_window));
        runtime->layout_window = NULL;
    }
    if (runtime->window_catalogue_window != NULL) {
        gtk_window_destroy(GTK_WINDOW(runtime->window_catalogue_window));
        runtime->window_catalogue_window = NULL;
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

    if (runtime->css_provider != NULL) {
        g_object_unref(runtime->css_provider);
        runtime->css_provider = NULL;
    }
}
