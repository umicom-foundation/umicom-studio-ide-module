/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_ecosystem_contribution.h
 *
 * PURPOSE:
 *   Declare Studio-only placement for the Framework package, SDK, extension
 *   and component workbench. Operational behaviour remains in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_H
#define UMICOM_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "umicom/developer/ecosystem/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_API_VERSION 1U

/**
 * Represent the studio developer ecosystem command contribution data shared with callers
 * of this public contract.
 */
typedef struct UmiStudioDeveloperEcosystemCommandContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu;
    const char *group;
    int position;
    bool enabled;
} UmiStudioDeveloperEcosystemCommandContribution;

/**
 * Represent the studio developer ecosystem view contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioDeveloperEcosystemViewContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *view_id;
    const char *label;
    const char *framework_contract;
    const char *region;
    int position;
    bool visible;
    bool closable;
} UmiStudioDeveloperEcosystemViewContribution;

/**
 * Return the number of records represented by studio developer ecosystem command without
 * changing their state.
 */
size_t umi_studio_developer_ecosystem_command_count(void);
/**
 * Find studio developer ecosystem command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_at(size_t index);
/**
 * Find studio developer ecosystem command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_find(const char *framework_command_id);
/**
 * Return the number of records represented by studio developer ecosystem view without
 * changing their state.
 */
size_t umi_studio_developer_ecosystem_view_count(void);
/**
 * Find studio developer ecosystem view while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_at(size_t index);
/**
 * Find studio developer ecosystem view while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_find(const char *view_id);
/**
 * Initialise studio developer ecosystem from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_ecosystem_create(
    UmiEcosystemRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_H */
