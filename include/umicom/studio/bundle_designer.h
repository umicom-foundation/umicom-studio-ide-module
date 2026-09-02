/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/bundle_designer.h
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
#ifndef UMICOM_STUDIO_BUNDLE_DESIGNER_H
#define UMICOM_STUDIO_BUNDLE_DESIGNER_H

#include "umicom/studio/dependency_centre.h"
#include "umicom/studio/package_project.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio bundle designer data shared with callers of this public contract.
 */
typedef struct UmiStudioBundleDesigner {
    UmiPortableBundlePlan plan;
} UmiStudioBundleDesigner;

/**
 * Initialise studio bundle designer from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_bundle_designer_init(
    UmiStudioBundleDesigner *designer,
    const UmiPackageManifest *manifest,
    const char *platform_id,
    const char *staging_root,
    const char *output_path);
/**
 * Provide the studio bundle designer apply dependencies operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_bundle_designer_apply_dependencies(
    UmiStudioBundleDesigner *designer,
    const UmiStudioDependencyCentre *dependencies);
/**
 * Provide the studio bundle designer ready operation used by this module and its client
 * applications.
 */
int umi_studio_bundle_designer_ready(
    const UmiStudioBundleDesigner *designer);

#ifdef __cplusplus
}
#endif
#endif
