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
/* Umicom Studio IDE | Compiler and Polyglot Centre v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_centre.h"
#include "umicom/studio/compiler_views.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
