/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/build_configurations.inc
 * PURPOSE: Use Framework configuration storage while preserving Studio workspace and job ownership.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

UmiStatus UmiStudioBuildConfigurationContextRead(UmiStudioServices *services,
                                                 UmiStudioBuildConfigurationContext *out)
{
    if (services == NULL || out == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    if (UmiStudioBuildBusy(umi_studio_services_build(services)) ||
        UmiStudioDebuggerNativeBusy(umi_studio_services_debugger(services)))
        return UMI_STATUS_BUSY;
    if (services->build_profile_storage_generation == 0U)
        return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiWorkspaceGraphSnapshot workspace;
    UmiStatus status = umi_workspace_graph_snapshot(services->workspace, &workspace);
    if (status != UMI_STATUS_OK)
        return status;
    if (!workspace.open)
        return UMI_STATUS_INVALID_STATE;
    if (strlen(workspace.root) >= sizeof out->root)
        return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiStudioBuildConfigurationContext candidate = {0};
    candidate.workspace_revision = workspace.revision;
    candidate.storage_generation = services->build_profile_storage_generation;
    strcpy(candidate.root, workspace.root);
    *out = candidate;
    return UMI_STATUS_OK;
}
static UmiStatus StudioBuildConfigurationCheck(UmiStudioServices *services,
                                               const UmiStudioBuildConfigurationContext *context)
{
    if (context == NULL || memchr(context->root, '\0', sizeof context->root) == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioBuildConfigurationContext current;
    UmiStatus status = UmiStudioBuildConfigurationContextRead(services, &current);
    if (status != UMI_STATUS_OK)
        return status;
    return current.workspace_revision == context->workspace_revision &&
                   current.storage_generation == context->storage_generation &&
                   umi_path_equal(current.root, context->root)
               ? UMI_STATUS_OK
               : UMI_STATUS_INVALID_STATE;
}
UmiStatus UmiStudioBuildConfigurationsCapture(UmiStudioServices *services,
                                              const UmiStudioBuildConfigurationContext *context,
                                              UmiBuildConfigurationCatalogue *out)
{
    UmiStatus status = StudioBuildConfigurationCheck(services, context);
    return status == UMI_STATUS_OK
               ? UmiBuildConfigurationCapture(BuildProfileServer(services), context->root, out)
               : status;
}
UmiStatus UmiStudioBuildConfigurationLoad(UmiStudioServices *services,
                                          const UmiStudioBuildConfigurationContext *context,
                                          const char *name, uint64_t revision, UmiBuildProfile *out)
{
    UmiStatus status = StudioBuildConfigurationCheck(services, context);
    return status == UMI_STATUS_OK ? UmiBuildConfigurationLoad(BuildProfileServer(services),
                                                               context->root, name, revision, out)
                                   : status;
}
UmiStatus UmiStudioBuildConfigurationSave(UmiStudioServices *services,
                                          const UmiStudioBuildConfigurationContext *context,
                                          const char *name, const UmiBuildProfile *profile,
                                          uint64_t revision, uint64_t *out_revision)
{
    UmiStatus status = StudioBuildConfigurationCheck(services, context);
    if (status == UMI_STATUS_OK)
        status = umi_build_profile_validate(profile, NULL, 0U);
    if (status == UMI_STATUS_OK && !umi_path_equal(context->root, profile->source_directory))
        status = UMI_STATUS_INVALID_ARGUMENT;
    return status == UMI_STATUS_OK ? UmiBuildConfigurationSave(BuildProfileServer(services), name,
                                                               profile, revision, out_revision)
                                   : status;
}

/* Library management uses the same captured workspace/storage authority as load
 * and save. These operations never install a profile into the build service. */
UmiStatus UmiStudioBuildConfigurationRename(UmiStudioServices *services,
    const UmiStudioBuildConfigurationContext *context, const char *name, const char *replacement,
    uint64_t revision, uint64_t *out_revision)
{
    UmiStatus status = StudioBuildConfigurationCheck(services, context);
    return status == UMI_STATUS_OK
        ? UmiBuildConfigurationRename(BuildProfileServer(services), context->root, name, replacement, revision, out_revision)
        : status;
}
UmiStatus UmiStudioBuildConfigurationRemove(UmiStudioServices *services,
    const UmiStudioBuildConfigurationContext *context, const char *name,
    uint64_t revision, uint64_t *out_revision)
{
    UmiStatus status = StudioBuildConfigurationCheck(services, context);
    return status == UMI_STATUS_OK
        ? UmiBuildConfigurationRemove(BuildProfileServer(services), context->root, name, revision, out_revision)
        : status;
}
