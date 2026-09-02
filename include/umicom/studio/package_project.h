/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/package_project.h
 *
 * PURPOSE:
 *   Build Windows and Linux package manifests for the Studio product.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PACKAGE_PROJECT_H
#define UMICOM_STUDIO_PACKAGE_PROJECT_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio package project data shared with callers of this public contract.
 */
typedef struct UmiStudioPackageProject {
    UmiPackageManifest windows;
    UmiPackageManifest linux;
} UmiStudioPackageProject;

/**
 * Initialise studio package project from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_package_project_init(UmiStudioPackageProject *project,
                                              const char *version);
/**
 * Provide the studio package project manifest operation used by this module and its client
 * applications.
 */
const UmiPackageManifest *umi_studio_package_project_manifest(
    const UmiStudioPackageProject *project,
    UmiInstallerPlatform platform);
/**
 * Provide the studio package project ready operation used by this module and its client
 * applications.
 */
int umi_studio_package_project_ready(const UmiStudioPackageProject *project);

#ifdef __cplusplus
}
#endif
#endif
