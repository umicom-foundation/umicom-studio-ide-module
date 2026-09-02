/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_CENTRE_H
#define UMICOM_STUDIO_COMPILER_CENTRE_H
#include "umicom/studio/compiler_settings.h"
#include "umicom/studio/toolchain_centre.h"
/**
 * Represent the studio compiler centre data shared with callers of this public contract.
 */
typedef struct UmiStudioCompilerCentre UmiStudioCompilerCentre;
/**
 * Represent the studio compiler centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioCompilerCentreSnapshot { UmiCompilerCentreSnapshot compiler; UmiStudioCompilerSettings settings; char active_view[UMI_COMPILER_ID_CAPACITY]; bool toolchain_synchronised; bool project_open; uint64_t revision; } UmiStudioCompilerCentreSnapshot;
/**
 * Initialise studio compiler centre from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_compiler_centre_create(UmiStudioToolchainCentre *toolchains,UmiStudioCompilerCentre **out_centre);
/**
 * Release or reset state held by studio compiler centre so the same storage can be reused
 * safely.
 */
void umi_studio_compiler_centre_destroy(UmiStudioCompilerCentre *centre);
/**
 * Provide the studio compiler centre register defaults operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_compiler_centre_register_defaults(UmiStudioCompilerCentre *centre,const char *gcc,const char *clang,const char *rustc,const char *zig,const char *umicc,const char *target);
/**
 * Provide the studio compiler centre sync toolchain operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_sync_toolchain(UmiStudioCompilerCentre *centre);
/**
 * Provide the studio compiler centre open project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_open_project(UmiStudioCompilerCentre *centre,const UmiCompilerProjectManifest *manifest);
/**
 * Provide the studio compiler centre activate view operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_activate_view(UmiStudioCompilerCentre *centre,const char *view_id);
/**
 * Provide the studio compiler centre plan unit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_centre_plan_unit(UmiStudioCompilerCentre *centre,const char *project_id,const char *unit_id,UmiCompilerPlan *out_plan);
/**
 * Provide the studio compiler centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_compiler_centre_snapshot(const UmiStudioCompilerCentre *centre,UmiStudioCompilerCentreSnapshot *out_snapshot);
/**
 * Provide the studio compiler centre engine operation used by this module and its client
 * applications.
 */
UmiCompilerCentre *umi_studio_compiler_centre_engine(UmiStudioCompilerCentre *centre);
#endif
