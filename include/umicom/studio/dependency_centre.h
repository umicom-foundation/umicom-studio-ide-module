/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/dependency_centre.h
 *
 * PURPOSE:
 *   Classify runtime files that a Studio portable bundle must include.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEPENDENCY_CENTRE_H
#define UMICOM_STUDIO_DEPENDENCY_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio dependency centre data shared with callers of this public contract.
 */
typedef struct UmiStudioDependencyCentre {
    UmiRuntimeDependencyReport report;
} UmiStudioDependencyCentre;

/**
 * Initialise studio dependency centre from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_dependency_centre_init(UmiStudioDependencyCentre *centre);
/**
 * Add studio dependency centre only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_dependency_centre_add(
    UmiStudioDependencyCentre *centre,
    const char *name,
    const char *resolved_path);
/**
 * Provide the studio dependency centre classify operation used by this module and its
 * client applications.
 */
UmiDependencyDisposition umi_studio_dependency_centre_classify(
    const char *name);
/**
 * Provide the studio dependency centre ready operation used by this module and its client
 * applications.
 */
int umi_studio_dependency_centre_ready(
    const UmiStudioDependencyCentre *centre);

#ifdef __cplusplus
}
#endif
#endif
