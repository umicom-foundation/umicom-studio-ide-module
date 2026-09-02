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

/**
 * Represent the studio web api workbench command contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioWebApiWorkbenchCommandContribution {
    uint32_t structure_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu;
    const char *group;
    int position;
    bool enabled;
} UmiStudioWebApiWorkbenchCommandContribution;

/**
 * Represent the studio web api workbench view contribution data shared with callers of
 * this public contract.
 */
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

/**
 * Return the number of records represented by studio web api workbench command without
 * changing their state.
 */
size_t umi_studio_web_api_workbench_command_count(void);
/**
 * Find studio web api workbench command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_at(size_t index);
/**
 * Find studio web api workbench command while leaving the underlying catalogue or model
 * owned by this module.
 */
const UmiStudioWebApiWorkbenchCommandContribution *
umi_studio_web_api_workbench_command_find(const char *framework_command_id);
/**
 * Return the number of records represented by studio web api workbench view without
 * changing their state.
 */
size_t umi_studio_web_api_workbench_view_count(void);
/**
 * Find studio web api workbench view while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_at(size_t index);
/**
 * Find studio web api workbench view while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioWebApiWorkbenchViewContribution *
umi_studio_web_api_workbench_view_find(const char *view_id);
/**
 * Initialise studio web api workbench from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_web_api_workbench_create(
    UmiWebWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_WEB_API_WORKBENCH_CONTRIBUTION_H */
