/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/bundle_designer.c
 *
 * PURPOSE:
 *   Compose Studio package files and runtime dependencies into a portable bundle.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bundle_designer.h"

/*
 * Initialise studio bundle designer from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_bundle_designer_init(
    UmiStudioBundleDesigner *designer,
    const UmiPackageManifest *manifest,
    const char *platform_id,
    const char *staging_root,
    const char *output_path)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (designer == NULL || manifest == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_portable_bundle_init(
        &designer->plan, "umicom-studio-portable", platform_id, staging_root,
        output_path, manifest);
}

/*
 * Provide the studio bundle designer apply dependencies operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_bundle_designer_apply_dependencies(
    UmiStudioBundleDesigner *designer,
    const UmiStudioDependencyCentre *dependencies)
{
    size_t index;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (designer == NULL || dependencies == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    umi_runtime_dependency_report_init(&designer->plan.dependencies);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < dependencies->report.count; ++index) {
        const UmiRuntimeDependency *dependency =
            &dependencies->report.dependencies[index];
        status = umi_portable_bundle_add_dependency(
            &designer->plan, dependency->name, dependency->resolved_path,
            dependency->disposition, dependency->resolved);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return UMI_STATUS_OK;
}

/*
 * Provide the studio bundle designer ready operation used by this module and its client
 * applications.
 */
int umi_studio_bundle_designer_ready(
    const UmiStudioBundleDesigner *designer)
{
    return designer != NULL &&
           umi_portable_bundle_validate(&designer->plan) == UMI_STATUS_OK;
}
