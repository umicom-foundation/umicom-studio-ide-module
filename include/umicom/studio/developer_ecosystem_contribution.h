/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/developer_ecosystem_contribution.h
 *
 * PURPOSE:
 *   Declare Studio-only placement for the Framework package, SDK, extension
 *   and component workbench. Operational behaviour remains in Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
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

typedef struct UmiStudioDeveloperEcosystemCommandContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu;
    const char *group;
    int position;
    bool enabled;
} UmiStudioDeveloperEcosystemCommandContribution;

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

size_t umi_studio_developer_ecosystem_command_count(void);
const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_at(size_t index);
const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_find(const char *framework_command_id);
size_t umi_studio_developer_ecosystem_view_count(void);
const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_at(size_t index);
const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_find(const char *view_id);
UmiStatus umi_studio_developer_ecosystem_create(
    UmiEcosystemRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_H */
