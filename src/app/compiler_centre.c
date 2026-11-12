/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_centre.c
 *
 * PURPOSE:
 *   Implement the compiler centre behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_centre.h"
#include "umicom/studio/compiler_views.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Framework now owns coherent provider/profile replacement and checked snapshots; Studio keeps product defaults and view state. The previous implementation is retained for review. */
#if 0
struct UmiStudioCompilerCentre { UmiStudioToolchainCentre *toolchains; UmiCompilerCentre *compiler; UmiStudioCompilerSettings settings; char active_view[UMI_COMPILER_ID_CAPACITY]; bool toolchain_synchronised; bool project_open; uint64_t revision; };
/* Provide the register provider operation used by this module and its client applications. */
static UmiStatus register_provider(UmiCompilerRegistry *registry,UmiCompilerProvider provider) { UmiStatus status; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (!provider.descriptor.available) return UMI_STATUS_OK; status = umi_compiler_registry_register(registry,&provider); return status == UMI_STATUS_ALREADY_EXISTS ? UMI_STATUS_OK : status; }
/*
 * Initialise studio compiler centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_compiler_centre_create(UmiStudioToolchainCentre *toolchains,UmiStudioCompilerCentre **out_centre)
{ UmiStudioCompilerCentre *centre; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT; *out_centre = NULL; centre = (UmiStudioCompilerCentre *)calloc(1U,sizeof(*centre)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY; centre->compiler = (UmiCompilerCentre *)calloc(1U,sizeof(*centre->compiler)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre->compiler == NULL) { free(centre); return UMI_STATUS_OUT_OF_MEMORY; } centre->toolchains = toolchains; umi_compiler_centre_init(centre->compiler); umi_studio_compiler_settings_defaults(&centre->settings); (void)snprintf(centre->active_view,sizeof(centre->active_view),"compiler.overview"); centre->revision = 1U; *out_centre = centre; return UMI_STATUS_OK; }
/*
 * Release or reset state held by studio compiler centre so the same storage can be reused
 * safely.
 */
void umi_studio_compiler_centre_destroy(UmiStudioCompilerCentre *centre) { /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL) return; free(centre->compiler); free(centre); }
/*
 * Provide the studio compiler centre register defaults operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_compiler_centre_register_defaults(UmiStudioCompilerCentre *centre,const char *gcc,const char *clang,const char *rustc,const char *zig,const char *umicc,const char *target)
{
    UmiCompilerTarget parsed; const UmiCompilerProvider *selected; UmiCompilerProfile profile; UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = target != NULL && target[0] != '\0' ? umi_compiler_target_parse(target,&parsed) : umi_compiler_target_host(&parsed);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = register_provider(&centre->compiler->providers,umi_compiler_gcc_provider(gcc != NULL ? gcc : "",parsed.triple)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status == UMI_STATUS_OK) status = register_provider(&centre->compiler->providers,umi_compiler_clang_provider(clang != NULL ? clang : "",parsed.triple)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status == UMI_STATUS_OK) status = register_provider(&centre->compiler->providers,umi_compiler_rust_provider(rustc != NULL ? rustc : "",parsed.triple)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status == UMI_STATUS_OK) status = register_provider(&centre->compiler->providers,umi_compiler_zig_provider(zig != NULL ? zig : "",parsed.triple)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status == UMI_STATUS_OK) status = register_provider(&centre->compiler->providers,umi_compiler_umicc_provider(umicc != NULL ? umicc : "",parsed.triple)); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status != UMI_STATUS_OK) return status;
    selected = umi_compiler_registry_resolve(&centre->compiler->providers,UMI_COMPILER_LANGUAGE_C,true); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (selected == NULL) return UMI_STATUS_UNAVAILABLE; status = umi_compiler_profile_init(&profile,"host-debug","Host Debug",selected->descriptor.provider_id,UMI_COMPILER_LANGUAGE_C,&parsed); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status != UMI_STATUS_OK) return status; status = umi_compiler_centre_add_profile(centre->compiler,&profile); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status != UMI_STATUS_OK && status != UMI_STATUS_ALREADY_EXISTS) return status; centre->revision += 1U; return UMI_STATUS_OK;
}
/*
 * Provide the studio compiler centre sync toolchain operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_sync_toolchain(UmiStudioCompilerCentre *centre)
{
    const UmiToolchainProfile *toolchain; UmiCompilerProfile profile; UmiStatus status; size_t imported = 0U;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || centre->toolchains == NULL) return UMI_STATUS_INVALID_STATE;
    toolchain = umi_studio_toolchain_centre_selected_profile(centre->toolchains);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (toolchain == NULL) return UMI_STATUS_NOT_FOUND;
    status = umi_compiler_registry_import_toolchain(&centre->compiler->providers,toolchain,&imported);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK && status != UMI_STATUS_ALREADY_EXISTS) return status;
    status = umi_compiler_profile_from_toolchain(toolchain,"toolchain-selected",&profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_compiler_centre_add_profile(centre->compiler,&profile);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK && status != UMI_STATUS_ALREADY_EXISTS) return status;
    status = umi_compiler_centre_activate_profile(centre->compiler,"toolchain-selected");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    centre->toolchain_synchronised = true;
    centre->revision += imported + 1U;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio compiler centre open project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_open_project(UmiStudioCompilerCentre *centre,const UmiCompilerProjectManifest *manifest) { UmiStatus status; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT; status = umi_compiler_centre_add_manifest(centre->compiler,manifest); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (status == UMI_STATUS_OK) { centre->project_open = true; centre->revision += 1U; } return status; }
/*
 * Provide the studio compiler centre activate view operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_activate_view(UmiStudioCompilerCentre *centre,const char *view_id) { /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL || view_id == NULL) return UMI_STATUS_INVALID_ARGUMENT; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (umi_studio_compiler_views_find(view_id) == NULL) return UMI_STATUS_NOT_FOUND; (void)snprintf(centre->active_view,sizeof(centre->active_view),"%s",view_id); centre->revision += 1U; return UMI_STATUS_OK; }
/*
 * Provide the studio compiler centre plan unit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_plan_unit(UmiStudioCompilerCentre *centre,const char *project_id,const char *unit_id,UmiCompilerPlan *out_plan) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : umi_compiler_centre_plan_unit(centre->compiler,project_id,unit_id,out_plan); }
/*
 * Provide the studio compiler centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_compiler_centre_snapshot(const UmiStudioCompilerCentre *centre,UmiStudioCompilerCentreSnapshot *out_snapshot) { /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT; (void)memset(out_snapshot,0,sizeof(*out_snapshot)); umi_compiler_centre_snapshot(centre->compiler,&out_snapshot->compiler); out_snapshot->settings = centre->settings; (void)snprintf(out_snapshot->active_view,sizeof(out_snapshot->active_view),"%s",centre->active_view); out_snapshot->toolchain_synchronised = centre->toolchain_synchronised; out_snapshot->project_open = centre->project_open; out_snapshot->revision = centre->revision + out_snapshot->compiler.revision; return UMI_STATUS_OK; }
/*
 * Provide the studio compiler centre engine operation used by this module and its client
 * applications.
 */
UmiCompilerCentre *umi_studio_compiler_centre_engine(UmiStudioCompilerCentre *centre) { return centre == NULL ? NULL : centre->compiler; }

#endif
struct UmiStudioCompilerCentre {
    UmiStudioToolchainCentre *toolchains; /* borrowed; outlives this adapter */
    UmiCompilerCentre *compiler;         /* owned; stable address for engine borrowers */
    UmiStudioCompilerSettings settings;
    char active_view[UMI_COMPILER_ID_CAPACITY];
    bool toolchain_synchronised;
    bool project_open;
    uint64_t revision;
};

UmiStatus umi_studio_compiler_centre_create(UmiStudioToolchainCentre *toolchains,
                                           UmiStudioCompilerCentre **out_centre)
{
    if (out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    UmiStudioCompilerCentre *centre = calloc(1U,sizeof(*centre));
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    centre->compiler = calloc(1U,sizeof(*centre->compiler));
    if (centre->compiler == NULL) { free(centre); return UMI_STATUS_OUT_OF_MEMORY; }
    centre->toolchains = toolchains;
    umi_compiler_centre_init(centre->compiler);
    umi_studio_compiler_settings_defaults(&centre->settings);
    (void)strcpy(centre->active_view,"compiler.overview");
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}

void umi_studio_compiler_centre_destroy(UmiStudioCompilerCentre *centre)
{
    if (centre == NULL) return;
    free(centre->compiler);
    free(centre);
}

UmiStatus umi_studio_compiler_centre_register_defaults(UmiStudioCompilerCentre *centre,
    const char *gcc,const char *clang,const char *rustc,const char *zig,const char *umicc,const char *target)
{
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!umi_compiler_centre_storage_valid(centre->compiler)) return UMI_STATUS_INVALID_STATE;
    if (centre->revision == UINT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiCompilerTarget parsed;
    UmiStatus status = target != NULL && target[0] != '\0' ?
        umi_compiler_target_parse(target,&parsed) : umi_compiler_target_host(&parsed);
    if (status != UMI_STATUS_OK) return status;
    const UmiCompilerProvider supplied[] = {
        umi_compiler_gcc_provider(gcc != NULL ? gcc : "",parsed.triple),
        umi_compiler_clang_provider(clang != NULL ? clang : "",parsed.triple),
        umi_compiler_rust_provider(rustc != NULL ? rustc : "",parsed.triple),
        umi_compiler_zig_provider(zig != NULL ? zig : "",parsed.triple),
        umi_compiler_umicc_provider(umicc != NULL ? umicc : "",parsed.triple)
    };
    UmiCompilerProvider providers[5];
    size_t count = 0U;
    const UmiCompilerProvider *selected = NULL;
    for (size_t i = 0U; i < sizeof(supplied)/sizeof(supplied[0]); ++i) {
        if (supplied[i].revision == 0U) return UMI_STATUS_CAPACITY_EXCEEDED;
        if (!supplied[i].descriptor.available) continue;
        providers[count] = supplied[i];
        if (selected == NULL && umi_compiler_provider_supports(&providers[count],UMI_COMPILER_LANGUAGE_C))
            selected = &providers[count];
        ++count;
    }
    if (selected == NULL)
        selected = umi_compiler_registry_resolve(&centre->compiler->providers,UMI_COMPILER_LANGUAGE_C,true);
    if (selected == NULL) return UMI_STATUS_UNAVAILABLE;
    UmiCompilerProfile *profile = malloc(sizeof(*profile));
    if (profile == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_compiler_profile_init(profile,"host-debug","Host Debug",
        selected->descriptor.provider_id,UMI_COMPILER_LANGUAGE_C,&parsed);
    /* Framework stages all reusable registry/profile work. Studio retains only
     * its product defaults and presentation revision, without copying the engine. */
    if (status == UMI_STATUS_OK)
        status = umi_compiler_centre_configure_profile(centre->compiler,providers,count,profile,
                                                       centre->compiler->active_profile_id[0] == '\0');
    free(profile);
    if (status == UMI_STATUS_OK) centre->revision += 1U;
    return status;
}

UmiStatus umi_studio_compiler_centre_sync_toolchain(UmiStudioCompilerCentre *centre)
{
    if (centre == NULL || centre->toolchains == NULL) return UMI_STATUS_INVALID_STATE;
    if (centre->revision == UINT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    const UmiToolchainProfile *toolchain = umi_studio_toolchain_centre_selected_profile(centre->toolchains);
    if (toolchain == NULL) return UMI_STATUS_NOT_FOUND;
    UmiStatus status = umi_compiler_centre_sync_toolchain(centre->compiler,toolchain,"toolchain-selected");
    if (status == UMI_STATUS_OK) {
        centre->toolchain_synchronised = true;
        centre->revision += 1U;
    }
    return status;
}

UmiStatus umi_studio_compiler_centre_open_project(UmiStudioCompilerCentre *centre,
                                                const UmiCompilerProjectManifest *manifest)
{
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (centre->revision == UINT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiStatus status = umi_compiler_centre_add_manifest(centre->compiler,manifest);
    if (status == UMI_STATUS_OK) { centre->project_open = true; centre->revision += 1U; }
    return status;
}

UmiStatus umi_studio_compiler_centre_activate_view(UmiStudioCompilerCentre *centre,const char *view_id)
{
    if (centre == NULL || view_id == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (umi_studio_compiler_views_find(view_id) == NULL) return UMI_STATUS_NOT_FOUND;
    if (centre->revision == UINT64_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    (void)snprintf(centre->active_view,sizeof(centre->active_view),"%s",view_id);
    centre->revision += 1U;
    return UMI_STATUS_OK;
}

UmiStatus umi_studio_compiler_centre_plan_unit_in_directory(UmiStudioCompilerCentre *centre,
    const char *project_id,const char *unit_id,const char *working_directory,UmiCompilerPlan *out_plan)
{
    return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT :
        umi_compiler_centre_plan_unit_in_directory(centre->compiler,project_id,unit_id,working_directory,out_plan);
}

UmiStatus umi_studio_compiler_centre_plan_unit(UmiStudioCompilerCentre *centre,
    const char *project_id,const char *unit_id,UmiCompilerPlan *out_plan)
{
    return umi_studio_compiler_centre_plan_unit_in_directory(centre,project_id,unit_id,".",out_plan);
}

static bool CompilerSnapshotOverlap(const void *owner,size_t owner_size,
                                    const void *output,size_t output_size)
{
    uintptr_t first = (uintptr_t)owner, second = (uintptr_t)output;
    return second >= first ? second-first < owner_size : first-second < output_size;
}

UmiStatus umi_studio_compiler_centre_snapshot(const UmiStudioCompilerCentre *centre,
                                            UmiStudioCompilerCentreSnapshot *out_snapshot)
{
    if (centre == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (CompilerSnapshotOverlap(centre,sizeof(*centre),out_snapshot,sizeof(*out_snapshot)) ||
        CompilerSnapshotOverlap(centre->compiler,sizeof(*centre->compiler),out_snapshot,sizeof(*out_snapshot)))
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioCompilerCentreSnapshot candidate = {0};
    UmiStatus status = umi_compiler_centre_snapshot_checked(centre->compiler,&candidate.compiler);
    if (status != UMI_STATUS_OK) return status;
    candidate.settings = centre->settings;
    (void)memcpy(candidate.active_view,centre->active_view,sizeof(candidate.active_view));
    candidate.toolchain_synchronised = centre->toolchain_synchronised;
    candidate.project_open = centre->project_open;
    /* The historical aggregate is an observation hint, not a unique change ID.
     * Saturate its sum; retain the exact Framework revision in the nested snapshot. */
    candidate.revision = UINT64_MAX - centre->revision < candidate.compiler.revision ?
        UINT64_MAX : centre->revision + candidate.compiler.revision;
    *out_snapshot = candidate;
    return UMI_STATUS_OK;
}

UmiCompilerCentre *umi_studio_compiler_centre_engine(UmiStudioCompilerCentre *centre)
{
    return centre == NULL ? NULL : centre->compiler;
}
