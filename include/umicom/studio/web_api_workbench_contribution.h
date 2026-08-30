/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/web_api_workbench_contribution.h
 *
 * PURPOSE:
 *   Declare Studio-only command and view placement for the Framework-owned
 *   HTTP/API/Web/Cloud workbench. Operational behaviour remains in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_H
#define UMICOM_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "umicom/web/workbench/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_API_VERSION 1U

typedef struct UmiStudioWebApiWorkbenchCommandContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu;
    const char *group;
    int position;
    bool enabled;
} UmiStudioWebApiWorkbenchCommandContribution;

typedef struct UmiStudioWebApiWorkbenchViewContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *view_id;
    const char *label;
    const char *framework_contract;
    const char *region;
    int position;
    bool visible;
    bool closable;
} UmiStudioWebApiWorkbenchViewContribution;

size_t umi_studio_web_api_workbench_command_count(void);
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_at(size_t index);
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_find(const char *framework_command_id);
size_t umi_studio_web_api_workbench_view_count(void);
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_at(size_t index);
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_find(const char *view_id);
UmiStatus umi_studio_web_api_workbench_create(
    UmiWebWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_H */
